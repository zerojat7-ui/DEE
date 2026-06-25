/*
 * character_simulation.c — DEE 성격 비교 시뮬레이션
 * 
 * 3명의 캐릭터(탐험가, 전략가, 수호자)에게 동일한 자극을 주고
 * 각각의 감정 변화와 능동 발화를 관찰합니다.
 * 
 * 컴파일:
 *   gcc -O2 -o character_simulation \
 *       character_simulation.c \
 *       ../kc_hormone.c ../kc_persona.c ../kc_vision.c ../kc_active_ai.c \
 *       -lm
 * 
 * 실행:
 *   ./character_simulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // sleep()
#include "kc_hormone.h"
#include "kc_persona.h"
#include "kc_active_ai.h"

/* ── 캐릭터 구조체 ─────────────────────────────────────── */
typedef struct {
    char name[32];
    KcEmotionEngine *dee;
    const KcPersonaProfile *persona;
    int active_fire_count;
} Character;

/* ── 캐릭터 생성 ────────────────────────────────────────── */
Character* create_character(const char *name, const char *persona_code) {
    Character *ch = malloc(sizeof(Character));
    strncpy(ch->name, name, 31);
    ch->dee = kc_dee_create();
    ch->persona = kc_persona_find(persona_code);
    kc_persona_apply(ch->dee, persona_code);
    ch->active_fire_count = 0;
    
    // Bio Rhythm 탄생 시각 초기화 (시뮬레이션 시작을 t=0으로)
    ch->dee->bio.t_elapsed = 0.0;
    ch->dee->bio.current = ch->dee->bio.amplitude * sinf(0) + ch->dee->bio.offset;
    
    return ch;
}

/* ── 캐릭터 감정 상태를 ASCII 바 그래프로 출력 ────────── */
void print_emotion_bars(Character *ch) {
    printf("\n┌─ %s (%s) ─────────────────────────────┐\n", 
           ch->name, ch->persona->code);
    
    const char *emojis[] = {"😡", "😨", "😊", "😢", "🤔", "🤢", "😲", "🤝"};
    
    for (int j = 0; j < KC_EMO_COUNT; j++) {
        int bars = (int)(ch->dee->emo.e[j] * 20);
        printf("│ %s %-8s ", emojis[j], KC_EMOTION_NAMES[j]);
        for (int b = 0; b < 20; b++) {
            printf("%s", b < bars ? "█" : "░");
        }
        printf(" %.2f\n", ch->dee->emo.e[j]);
    }
    
    // 호르몬 상위 2종 표시
    int h1 = 0, h2 = 1;
    for (int i = 0; i < KC_HOR_COUNT; i++) {
        float r1 = ch->dee->hor.level[h1] / ch->dee->hor.h_max[h1];
        float r2 = ch->dee->hor.level[h2] / ch->dee->hor.h_max[h2];
        float ri = ch->dee->hor.level[i] / ch->dee->hor.h_max[i];
        if (ri > r1) { h2 = h1; h1 = i; }
        else if (i != h1 && ri > r2) { h2 = i; }
    }
    printf("│ 호르몬: %s(%.2f) %s(%.2f)\n",
           KC_HORMONE_NAMES[h1], ch->dee->hor.level[h1],
           KC_HORMONE_NAMES[h2], ch->dee->hor.level[h2]);
    
    // 능동 발화 상태
    int active = kc_dee_check_active(ch->dee);
    if (active) {
        ch->active_fire_count++;
        char prompt[2048];
        kc_dee_to_prompt(ch->dee, prompt, sizeof(prompt));
        printf("│ 🔥 능동발화! (총 %d회)\n", ch->active_fire_count);
        // 첫 줄만 출력
        char *first_line = strtok(prompt, "\n");
        if (first_line) printf("│   %s\n", first_line);
    }
    
    printf("└─────────────────────────────────────────────┘\n");
}

/* ── 캐릭터 상태 출력 ──────────────────────────────────── */
void print_all_characters(Character **chars, int count) {
    printf("\n═══════════════════════════════════════════════════════\n");
    printf("  시뮬레이션 시간: %.0f초\n", chars[0]->dee->t_real);
    printf("═══════════════════════════════════════════════════════\n");
    for (int i = 0; i < count; i++) {
        print_emotion_bars(chars[i]);
    }
}

/* ── 자극 주입 ───────────────────────────────────────────── */
void apply_stimulus(Character *ch, const char *desc, float impact, float threat, float fairness, float social) {
    kc_dee_stimulus(ch->dee, desc, impact, threat, fairness, 0.7f, social);
}

/* ── 메인 ─────────────────────────────────────────────────── */
int main() {
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║  🧪 DEE 성격 비교 시뮬레이션                         ║\n");
    printf("║  3명의 캐릭터에게 동일한 자극을 주고 반응을 관찰합니다 ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");

    // 1. 캐릭터 생성 (3종 성격)
    Character *chars[3];
    chars[0] = create_character("탐험가", KP_탐험가);  // KP01
    chars[1] = create_character("전략가", KP_전략가);  // KP02
    chars[2] = create_character("수호자", KP_수호자);  // KP09

    printf("\n✅ 캐릭터 3명 생성 완료!\n");
    printf("   - 탐험가 (KP01): 활동·직관·공감·탐색\n");
    printf("   - 전략가 (KP02): 성찰·직관·논리·계획\n");
    printf("   - 수호자 (KP09): 성찰·현실·공감·계획\n");

    // 2. 시뮬레이션 루프
    const int TOTAL_STEPS = 30;
    const float DT = 1.0f;  // 1초 간격

    for (int step = 0; step < TOTAL_STEPS; step++) {
        // 2-1. 모든 캐릭터 tick 업데이트
        for (int i = 0; i < 3; i++) {
            kc_dee_tick(chars[i]->dee, DT);
        }

        // 2-2. 특정 스텝에 자극 주입
        if (step == 2) {
            // Step 2: 칭찬 (긍정 자극)
            printf("\n💬 [%d초] 칭찬: \"정말 잘했어!\"\n", step);
            for (int i = 0; i < 3; i++) {
                apply_stimulus(chars[i], "칭찬", 0.8f, 0.0f, 0.9f, 0.7f);
            }
        }
        else if (step == 8) {
            // Step 8: 위협 (부정 자극)
            printf("\n💬 [%d초] 위협: \"이대로면 위험해!\"\n", step);
            for (int i = 0; i < 3; i++) {
                apply_stimulus(chars[i], "위협", -0.3f, 0.8f, 0.2f, 0.3f);
            }
        }
        else if (step == 15) {
            // Step 15: 반복 질문 (짜증 유발)
            printf("\n💬 [%d초] 반복질문: \"왜? 왜? 왜?\" (3회 반복)\n", step);
            for (int i = 0; i < 3; i++) {
                apply_stimulus(chars[i], "반복질문", 0.1f, 0.2f, 0.4f, 0.5f);
                // 짜증 누적을 위해 추가 자극
                kc_dee_stimulus(chars[i]->dee, "반복질문2", 0.1f, 0.2f, 0.4f, 0.5f);
                kc_dee_stimulus(chars[i]->dee, "반복질문3", 0.1f, 0.2f, 0.4f, 0.5f);
            }
        }
        else if (step == 22) {
            // Step 22: 긍정적 사회적 상호작용
            printf("\n💬 [%d초] 따뜻한 말: \"너를 믿어. 함께하자.\"\n", step);
            for (int i = 0; i < 3; i++) {
                apply_stimulus(chars[i], "신뢰", 0.6f, 0.0f, 0.8f, 0.9f);
            }
        }

        // 2-3. 상태 출력 (5초마다)
        if (step % 5 == 0 || step == TOTAL_STEPS - 1) {
            print_all_characters(chars, 3);
            sleep(1);  // 1초 대기 (사람이 읽을 시간)
        }
    }

    // 3. 최종 요약
    printf("\n═══════════════════════════════════════════════════════\n");
    printf("  📊 시뮬레이션 요약\n");
    printf("═══════════════════════════════════════════════════════\n");
    for (int i = 0; i < 3; i++) {
        printf("  %s: 능동발화 %d회\n", chars[i]->name, chars[i]->active_fire_count);
    }

    // 4. 정리
    for (int i = 0; i < 3; i++) {
        kc_dee_destroy(chars[i]->dee);
        free(chars[i]);
    }

    printf("\n✅ 시뮬레이션 완료!\n");
    return 0;
}
