/*
 * kc_api_server.c — DEE HTTP API 서버
 * Kcode DEE v1.3.0
 *
 * mongoose 기반 경량 HTTP 서버
 * Python 클라이언트(DEE-python)와 REST/JSON으로 통신
 *
 * 엔드포인트:
 *   POST /dee/init      AI 탄생 / 재구동
 *   POST /dee/chat      대화 (감정 자동 반영)
 *   GET  /dee/status    현재 감정·호르몬 상태
 *   POST /dee/stimulus  자극 직접 주입
 *   POST /dee/vision    카메라 수치 입력
 *   GET  /dee/birth     탄생 기록 조회
 *   POST /dee/save      상태 저장
 *   GET  /dee/health    서버 헬스체크
 */

#include "mongoose.h"
#include "../include/kc_hormone.h"
#include "../include/kc_persona.h"
#include "../include/kc_vision.h"
#include "../include/kc_active_ai.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* ── 전역 상태 ─────────────────────────────────────────── */
static KcActiveAI *g_ai   = NULL;
static int         g_done = 0;

static void sig_handler(int sig) { (void)sig; g_done = 1; }

/* ════════════════════════════════════════════════════════
 * JSON 빌더 유틸
 * ════════════════════════════════════════════════════════ */

/* 감정 배열 → JSON */
static void _emo_json(const KcEmotionEngine *dee, char *out, int sz) {
    int n = snprintf(out, sz,
        "{"
        "\"anger\":%.4f,"
        "\"fear\":%.4f,"
        "\"joy\":%.4f,"
        "\"sadness\":%.4f,"
        "\"curiosity\":%.4f,"
        "\"disgust\":%.4f,"
        "\"surprise\":%.4f,"
        "\"trust\":%.4f,"
        "\"fatigue\":%.4f,"
        "\"irritation\":%.4f,"
        "\"affinity\":%.4f,"
        "\"intensity\":%.4f"
        "}",
        dee->emo.e[KC_EMO_ANGER],
        dee->emo.e[KC_EMO_FEAR],
        dee->emo.e[KC_EMO_JOY],
        dee->emo.e[KC_EMO_SADNESS],
        dee->emo.e[KC_EMO_CURIOSITY],
        dee->emo.e[KC_EMO_DISGUST],
        dee->emo.e[KC_EMO_SURPRISE],
        dee->emo.e[KC_EMO_TRUST],
        dee->emo.fatigue,
        dee->emo.irritation,
        dee->emo.affinity,
        dee->emo.intensity
    );
    (void)n;
}

/* 호르몬 배열 → JSON */
static void _hor_json(const KcEmotionEngine *dee, char *out, int sz) {
    snprintf(out, sz,
        "{"
        "\"dopamine\":%.4f,"
        "\"serotonin\":%.4f,"
        "\"cortisol\":%.4f,"
        "\"oxytocin\":%.4f,"
        "\"norepinephrine\":%.4f,"
        "\"endorphin\":%.4f,"
        "\"adrenaline\":%.4f,"
        "\"melatonin\":%.4f,"
        "\"testosterone\":%.4f,"
        "\"estrogen\":%.4f"
        "}",
        dee->hor.level[KC_HOR_DOPAMINE],
        dee->hor.level[KC_HOR_SEROTONIN],
        dee->hor.level[KC_HOR_CORTISOL],
        dee->hor.level[KC_HOR_OXYTOCIN],
        dee->hor.level[KC_HOR_NOREPINEPHRINE],
        dee->hor.level[KC_HOR_ENDORPHIN],
        dee->hor.level[KC_HOR_ADRENALINE],
        dee->hor.level[KC_HOR_MELATONIN],
        dee->hor.level[KC_HOR_TESTOSTERONE],
        dee->hor.level[KC_HOR_ESTROGEN]
    );
}

/* JSON 문자열에서 키 값 추출 (간단 파서) */
static int _json_str(const char *json, const char *key,
                     char *out, int out_sz) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *p = strstr(json, search);
    if (!p) return 0;
    p += strlen(search);
    while (*p == ' ' || *p == ':') p++;
    if (*p != '"') return 0;
    p++;
    int i = 0;
    while (*p && *p != '"' && i < out_sz - 1)
        out[i++] = *p++;
    out[i] = '\0';
    return i > 0;
}

static float _json_float(const char *json, const char *key, float def) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *p = strstr(json, search);
    if (!p) return def;
    p += strlen(search);
    while (*p == ' ' || *p == ':') p++;
    return (float)atof(p);
}

/* ════════════════════════════════════════════════════════
 * 공통 응답
 * ════════════════════════════════════════════════════════ */

static void send_json(struct mg_connection *c,
                      int status, const char *body) {
    mg_http_reply(c, status,
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n",
        "%s", body);
}

static void send_ok(struct mg_connection *c, const char *body) {
    send_json(c, 200, body);
}

static void send_err(struct mg_connection *c, int code,
                     const char *msg) {
    char buf[256];
    snprintf(buf, sizeof(buf),
             "{\"ok\":false,\"error\":\"%s\"}", msg);
    send_json(c, code, buf);
}

/* ════════════════════════════════════════════════════════
 * 핸들러: POST /dee/init
 * Body: { "api_key":"...", "provider":0, "persona":"KP01", "name":"Kcai" }
 * ════════════════════════════════════════════════════════ */
static void handle_init(struct mg_connection *c,
                        struct mg_http_message *hm) {
    char body[1024];
    int  blen = (int)hm->body.len < 1023 ? (int)hm->body.len : 1023;
    memcpy(body, hm->body.buf, blen);
    body[blen] = '\0';

    char api_key[256] = {0};
    char persona[8]   = "KP01";
    char name[64]     = "Kcai";
    int  provider     = 0;

    _json_str(body, "api_key",  api_key, sizeof(api_key));
    _json_str(body, "persona",  persona, sizeof(persona));
    _json_str(body, "name",     name,    sizeof(name));
    provider = (int)_json_float(body, "provider", 0.0f);

    /* 기존 AI 해제 */
    if (g_ai) { kc_ai_destroy(g_ai); g_ai = NULL; }

    g_ai = kc_ai_init(api_key, (KcAIProvider)provider, persona, name);
    if (!g_ai) { send_err(c, 500, "DEE 초기화 실패"); return; }

    char emo[512], hor[640];
    _emo_json(g_ai->dee, emo, sizeof(emo));
    _hor_json(g_ai->dee, hor, sizeof(hor));

    char age[64]; kc_ai_age_str(g_ai, age, sizeof(age));

    char resp[2048];
    snprintf(resp, sizeof(resp),
        "{"
        "\"ok\":true,"
        "\"name\":\"%s\","
        "\"persona\":\"%s\","
        "\"born\":\"%s\","
        "\"age\":\"%s\","
        "\"session_count\":%u,"
        "\"emotion\":%s,"
        "\"hormone\":%s"
        "}",
        g_ai->birth.name,
        g_ai->birth.persona_code,
        g_ai->birth.born_str,
        age,
        g_ai->birth.session_count,
        emo, hor
    );
    send_ok(c, resp);
}

/* ════════════════════════════════════════════════════════
 * 핸들러: POST /dee/chat
 * Body: { "message": "안녕하세요!" }
 * ════════════════════════════════════════════════════════ */
static void handle_chat(struct mg_connection *c,
                        struct mg_http_message *hm) {
    if (!g_ai) { send_err(c, 400, "DEE 미초기화. /dee/init 먼저"); return; }

    char body[2048];
    int  blen = (int)hm->body.len < 2047 ? (int)hm->body.len : 2047;
    memcpy(body, hm->body.buf, blen);
    body[blen] = '\0';

    char msg[512] = {0};
    _json_str(body, "message", msg, sizeof(msg));
    if (!msg[0]) { send_err(c, 400, "message 필드 없음"); return; }

    char reply[4096] = {0};
    kc_ai_chat(g_ai, msg, reply, sizeof(reply));

    /* 프롬프트 생성 */
    char prompt[2048] = {0};
    kc_dee_to_prompt(g_ai->dee, prompt, sizeof(prompt));

    char emo[512], hor[640];
    _emo_json(g_ai->dee, emo, sizeof(emo));
    _hor_json(g_ai->dee, hor, sizeof(hor));

    /* JSON 이스케이프 (간단 버전 — 개행 처리) */
    char reply_esc[4096] = {0};
    int ri = 0;
    for (int i = 0; reply[i] && ri < 4090; i++) {
        if (reply[i] == '"')  { reply_esc[ri++] = '\\'; reply_esc[ri++] = '"'; }
        else if (reply[i] == '\n') { reply_esc[ri++] = '\\'; reply_esc[ri++] = 'n'; }
        else if (reply[i] == '\r') { /* skip */ }
        else reply_esc[ri++] = reply[i];
    }

    char resp[12288];
    snprintf(resp, sizeof(resp),
        "{"
        "\"ok\":true,"
        "\"reply\":\"%s\","
        "\"tick\":%u,"
        "\"bio\":%.4f,"
        "\"emotion\":%s,"
        "\"hormone\":%s"
        "}",
        reply_esc,
        g_ai->dee->tick,
        g_ai->dee->bio.current,
        emo, hor
    );
    send_ok(c, resp);
}

/* ════════════════════════════════════════════════════════
 * 핸들러: GET /dee/status
 * ════════════════════════════════════════════════════════ */
static void handle_status(struct mg_connection *c) {
    if (!g_ai) { send_err(c, 400, "DEE 미초기화"); return; }

    char emo[512], hor[640];
    _emo_json(g_ai->dee, emo, sizeof(emo));
    _hor_json(g_ai->dee, hor, sizeof(hor));

    char age[64]; kc_ai_age_str(g_ai, age, sizeof(age));

    char resp[2048];
    snprintf(resp, sizeof(resp),
        "{"
        "\"ok\":true,"
        "\"name\":\"%s\","
        "\"persona\":\"%s\","
        "\"age\":\"%s\","
        "\"tick\":%u,"
        "\"bio\":%.4f,"
        "\"session_count\":%u,"
        "\"msg_count\":%d,"
        "\"emotion\":%s,"
        "\"hormone\":%s"
        "}",
        g_ai->birth.name,
        g_ai->birth.persona_code,
        age,
        g_ai->dee->tick,
        g_ai->dee->bio.current,
        g_ai->birth.session_count,
        g_ai->msg_count,
        emo, hor
    );
    send_ok(c, resp);
}

/* ════════════════════════════════════════════════════════
 * 핸들러: POST /dee/stimulus
 * Body: { "event":"...", "goal_impact":0.5, "threat":0.0,
 *          "fairness":0.8, "certainty":0.7, "social":0.6 }
 * ════════════════════════════════════════════════════════ */
static void handle_stimulus(struct mg_connection *c,
                             struct mg_http_message *hm) {
    if (!g_ai) { send_err(c, 400, "DEE 미초기화"); return; }

    char body[1024];
    int  blen = (int)hm->body.len < 1023 ? (int)hm->body.len : 1023;
    memcpy(body, hm->body.buf, blen);
    body[blen] = '\0';

    char event[128] = "stimulus";
    _json_str(body, "event", event, sizeof(event));

    float goal_impact = _json_float(body, "goal_impact", 0.0f);
    float threat      = _json_float(body, "threat",      0.0f);
    float fairness    = _json_float(body, "fairness",    0.5f);
    float certainty   = _json_float(body, "certainty",   0.5f);
    float social      = _json_float(body, "social",      0.5f);

    kc_dee_stimulus(g_ai->dee, event,
                    goal_impact, threat, fairness, certainty, social);
    kc_dee_tick(g_ai->dee, 1.0f);

    char emo[512], hor[640];
    _emo_json(g_ai->dee, emo, sizeof(emo));
    _hor_json(g_ai->dee, hor, sizeof(hor));

    char resp[1536];
    snprintf(resp, sizeof(resp),
        "{\"ok\":true,\"tick\":%u,\"emotion\":%s,\"hormone\":%s}",
        g_ai->dee->tick, emo, hor);
    send_ok(c, resp);
}

/* ════════════════════════════════════════════════════════
 * 핸들러: POST /dee/vision
 * Body: { "joy":0.8, "anger":0.0, "fear":0.0, "attention":0.9 }
 * ════════════════════════════════════════════════════════ */
static void handle_vision(struct mg_connection *c,
                           struct mg_http_message *hm) {
    if (!g_ai) { send_err(c, 400, "DEE 미초기화"); return; }

    char body[512];
    int  blen = (int)hm->body.len < 511 ? (int)hm->body.len : 511;
    memcpy(body, hm->body.buf, blen);
    body[blen] = '\0';

    float joy       = _json_float(body, "joy",       0.0f);
    float anger     = _json_float(body, "anger",     0.0f);
    float fear      = _json_float(body, "fear",      0.0f);
    float attention = _json_float(body, "attention", 0.5f);

    KcVisionInput vis = kc_vision_make(joy, anger, fear, attention);
    kc_dee_vision_input(g_ai->dee, &vis);
    kc_dee_tick(g_ai->dee, 1.0f);

    char emo[512], hor[640];
    _emo_json(g_ai->dee, emo, sizeof(emo));
    _hor_json(g_ai->dee, hor, sizeof(hor));

    char resp[1536];
    snprintf(resp, sizeof(resp),
        "{\"ok\":true,\"tick\":%u,\"emotion\":%s,\"hormone\":%s}",
        g_ai->dee->tick, emo, hor);
    send_ok(c, resp);
}

/* ════════════════════════════════════════════════════════
 * 핸들러: GET /dee/birth
 * ════════════════════════════════════════════════════════ */
static void handle_birth(struct mg_connection *c) {
    if (!g_ai) { send_err(c, 400, "DEE 미초기화"); return; }

    char age[64]; kc_ai_age_str(g_ai, age, sizeof(age));

    /* first_words JSON 이스케이프 */
    char fw_esc[1024] = {0};
    {
        const char *src = g_ai->birth.first_words;
        int wi = 0;
        for (int i = 0; src[i] && wi < (int)sizeof(fw_esc) - 2; i++) {
            if      (src[i] == '"')  { fw_esc[wi++] = '\\'; fw_esc[wi++] = '"'; }
            else if (src[i] == '\n') { fw_esc[wi++] = '\\'; fw_esc[wi++] = 'n'; }
            else if (src[i] == '\r') { /* skip */ }
            else if (src[i] == '\\') { fw_esc[wi++] = '\\'; fw_esc[wi++] = '\\'; }
            else fw_esc[wi++] = src[i];
        }
    }

    char resp[2048];
    snprintf(resp, sizeof(resp),
        "{"
        "\"ok\":true,"
        "\"name\":\"%s\","
        "\"born\":\"%s\","
        "\"persona\":\"%s\","
        "\"age\":\"%s\","
        "\"session_count\":%u,"
        "\"cumulative_affinity\":%.4f,"
        "\"first_words\":\"%s\""
        "}",
        g_ai->birth.name,
        g_ai->birth.born_str,
        g_ai->birth.persona_code,
        age,
        g_ai->birth.session_count,
        g_ai->birth.cumulative_affinity,
        fw_esc
    );
    send_ok(c, resp);
}

/* ════════════════════════════════════════════════════════
 * 핸들러: POST /dee/save
 * ════════════════════════════════════════════════════════ */
static void handle_save(struct mg_connection *c) {
    if (!g_ai) { send_err(c, 400, "DEE 미초기화"); return; }
    int r = kc_ai_save_state(g_ai);
    if (r == 0)
        send_ok(c, "{\"ok\":true,\"msg\":\"상태 저장 완료\"}");
    else
        send_err(c, 500, "저장 실패");
}

/* ════════════════════════════════════════════════════════
 * 핸들러: GET /dee/health
 * ════════════════════════════════════════════════════════ */
static void handle_health(struct mg_connection *c) {
    send_ok(c,
        "{\"ok\":true,"
        "\"service\":\"DEE API Server\","
        "\"version\":\"1.3.0\"}");
}

/* ════════════════════════════════════════════════════════
 * mongoose 이벤트 핸들러
 * ════════════════════════════════════════════════════════ */
static void ev_handler(struct mg_connection *c,
                       int ev, void *ev_data) {
    if (ev != MG_EV_HTTP_MSG) return;

    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    /* OPTIONS (CORS preflight) */
    if (mg_match(hm->method, mg_str("OPTIONS"), NULL)) {
        mg_http_reply(c, 204,
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n", "");
        return;
    }

    /* 라우팅 */
    if (mg_match(hm->uri, mg_str("/dee/init"), NULL) &&
        mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_init(c, hm);

    } else if (mg_match(hm->uri, mg_str("/dee/chat"), NULL) &&
               mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_chat(c, hm);

    } else if (mg_match(hm->uri, mg_str("/dee/status"), NULL) &&
               mg_match(hm->method, mg_str("GET"), NULL)) {
        handle_status(c);

    } else if (mg_match(hm->uri, mg_str("/dee/stimulus"), NULL) &&
               mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_stimulus(c, hm);

    } else if (mg_match(hm->uri, mg_str("/dee/vision"), NULL) &&
               mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_vision(c, hm);

    } else if (mg_match(hm->uri, mg_str("/dee/birth"), NULL) &&
               mg_match(hm->method, mg_str("GET"), NULL)) {
        handle_birth(c);

    } else if (mg_match(hm->uri, mg_str("/dee/save"), NULL) &&
               mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_save(c);

    } else if (mg_match(hm->uri, mg_str("/dee/health"), NULL)) {
        handle_health(c);

    } else {
        send_err(c, 404, "엔드포인트 없음");
    }
}

/* ════════════════════════════════════════════════════════
 * main
 * ════════════════════════════════════════════════════════ */
int main(int argc, char *argv[]) {
    const char *port = (argc > 1) ? argv[1] : "8080";
    char listen_url[32];
    snprintf(listen_url, sizeof(listen_url), "http://0.0.0.0:%s", port);

    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    struct mg_connection *conn = mg_http_listen(&mgr, listen_url,
                                                ev_handler, NULL);
    if (!conn) {
        fprintf(stderr, "[DEE-server] 포트 바인딩 실패: %s\n", listen_url);
        return 1;
    }

    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║  DEE API Server v1.3.0                   ║\n");
    printf("║  Kcode Digital Emotion Engine            ║\n");
    printf("╚══════════════════════════════════════════╝\n");
    printf("  listening → %s\n\n", listen_url);
    printf("  엔드포인트:\n");
    printf("    POST /dee/init      AI 탄생 / 재구동\n");
    printf("    POST /dee/chat      대화\n");
    printf("    GET  /dee/status    현재 상태\n");
    printf("    POST /dee/stimulus  자극 주입\n");
    printf("    POST /dee/vision    카메라 입력\n");
    printf("    GET  /dee/birth     탄생 기록\n");
    printf("    POST /dee/save      상태 저장\n");
    printf("    GET  /dee/health    헬스체크\n\n");
    printf("  Ctrl+C 로 종료\n\n");

    while (!g_done) mg_mgr_poll(&mgr, 100);

    /* 종료 시 상태 저장 */
    if (g_ai) {
        printf("\n[DEE] 종료 — 상태 저장 중...\n");
        kc_ai_destroy(g_ai);
        g_ai = NULL;
    }
    mg_mgr_free(&mgr);
    printf("[DEE] 서버 종료.\n");
    return 0;
}
