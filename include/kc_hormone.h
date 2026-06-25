/*
 * kc_hormone.h — Digital Emotion Engine: 호르몬 동역학
 * Kcode DEE v1.0.1 (미연결 필드 수정)
 */
#ifndef KC_HORMONE_H
#define KC_HORMONE_H

#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KC_DEE_VERSION  "1.0.1"

typedef enum {
    KC_HOR_DOPAMINE       = 0,
    KC_HOR_SEROTONIN      = 1,
    KC_HOR_CORTISOL       = 2,
    KC_HOR_OXYTOCIN       = 3,
    KC_HOR_NOREPINEPHRINE = 4,
    KC_HOR_ENDORPHIN      = 5,
    KC_HOR_ADRENALINE     = 6,
    KC_HOR_MELATONIN      = 7,
    KC_HOR_TESTOSTERONE   = 8,
    KC_HOR_ESTROGEN       = 9,
    KC_HOR_COUNT          = 10
} KcHormoneType;

typedef enum {
    KC_EMO_ANGER     = 0,
    KC_EMO_FEAR      = 1,
    KC_EMO_JOY       = 2,
    KC_EMO_SADNESS   = 3,
    KC_EMO_CURIOSITY = 4,
    KC_EMO_DISGUST   = 5,
    KC_EMO_SURPRISE  = 6,
    KC_EMO_TRUST     = 7,
    KC_EMO_COUNT     = 8
} KcEmotionType;

typedef enum {
    KC_DRIVE_SURVIVE    = 0,
    KC_DRIVE_CONNECT    = 1,
    KC_DRIVE_GROW       = 2,
    KC_DRIVE_EXPRESS    = 3,
    KC_DRIVE_UNDERSTAND = 4,
    KC_DRIVE_CREATE     = 5,
    KC_DRIVE_COUNT      = 6
} KcDriveType;

typedef struct {
    float  amplitude;
    float  omega;
    float  phase;
    float  offset;
    float  current;
    double t_elapsed;
} KcBioRhythm;

typedef struct {
    float  level[KC_HOR_COUNT];
    float  lambda[KC_HOR_COUNT];
    float  h_max[KC_HOR_COUNT];
    float  input_buf[KC_HOR_COUNT];
    float  diffusion[KC_HOR_COUNT];
    float  tolerance[KC_HOR_COUNT];
} KcHormoneState;

typedef struct {
    float  e[KC_EMO_COUNT];
    float  e_prev[KC_EMO_COUNT];
    float  regulation;
    float  irritation;
    float  fatigue;
    float  affinity;
    float  intensity;
} KcEmotionState;

typedef struct {
    float  level[KC_DRIVE_COUNT];
    float  threshold[KC_DRIVE_COUNT];
    float  satisfaction[KC_DRIVE_COUNT];
    float  decay_rate[KC_DRIVE_COUNT];
} KcDriveState;

#define KC_GOAL_MAX     32
#define KC_GOAL_DESC    128

typedef struct {
    char     desc[KC_GOAL_DESC];
    float    priority;
    float    progress;
    int      active;
    uint32_t created_tick;
    uint32_t deadline_tick;
    float    reward;
} KcGoal;

typedef struct {
    KcGoal   st[KC_GOAL_MAX];
    KcGoal   lt[KC_GOAL_MAX];
    int      st_cnt;
    int      lt_cnt;
} KcGoalSystem;

#define KC_MEM_MAX      256
#define KC_MEM_STR      192

typedef struct {
    char   event[KC_MEM_STR];
    float  e_snap[KC_EMO_COUNT];
    float  h_snap[KC_HOR_COUNT];
    uint32_t tick;
    float  importance;
    uint8_t decay_class;
} KcEmotionMemory;

#define KC_EMOTION_PROMPT_MAX  2048
#define KC_EMOTION_LOG_MAX     64

typedef struct {
    KcHormoneState  hor;
    KcEmotionState  emo;
    KcBioRhythm     bio;
    KcDriveState    drive;
    KcGoalSystem    goals;
    KcEmotionMemory mem[KC_MEM_MAX];
    int             mem_cnt;
    uint32_t        tick;
    double          t_real;

    /* 능동 발화 */
    float  ai_temp_base;         /* 감정 기반 temperature — kc_ai_chat()에 반영 */
    void (*on_active)(const char *prompt, void *ud);
    void  *userdata;
    uint32_t last_fire_tick;     /* 능동 발화 쿨다운 (static 제거) */

    /* KcPersona 런타임 파라미터 — kc_persona_apply()가 설정 */
    float  persona_energy_recharge; /* 성찰형↑ Bio 회복 속도  */
    float  persona_social_drain;    /* 활동형↑ 사회적 에너지 소모 */
    float  persona_irritation_k;    /* 짜증 전환 계수          */

    /* 내부 로그 */
    char   log[KC_EMOTION_LOG_MAX][128];
    int    log_cnt;
} KcEmotionEngine;

/* API */
KcEmotionEngine *kc_dee_create(void);
void             kc_dee_destroy(KcEmotionEngine *dee);
void             kc_dee_reset(KcEmotionEngine *dee);
void             kc_dee_tick(KcEmotionEngine *dee, float dt);
void             kc_dee_stimulus(KcEmotionEngine *dee,
                                 const char *event_desc,
                                 float goal_impact,
                                 float threat,
                                 float fairness,
                                 float certainty,
                                 float social);
void kc_dee_affinity(KcEmotionEngine *dee, float delta);
int  kc_dee_goal_add(KcEmotionEngine *dee, const char *desc,
                     float priority, uint32_t deadline_ticks,
                     float reward, int is_longterm);
void kc_dee_goal_progress(KcEmotionEngine *dee, int idx,
                          float delta, int is_longterm);
void kc_dee_to_prompt(const KcEmotionEngine *dee, char *out, int out_sz);
int  kc_dee_check_active(KcEmotionEngine *dee);
void kc_dee_print(const KcEmotionEngine *dee);

extern const float KC_HOR_EMO_MATRIX[KC_HOR_COUNT][KC_EMO_COUNT];
extern const char *KC_HORMONE_NAMES[KC_HOR_COUNT];
extern const char *KC_EMOTION_NAMES[KC_EMO_COUNT];
extern const char *KC_DRIVE_NAMES[KC_DRIVE_COUNT];

#ifdef __cplusplus
}
#endif
#endif /* KC_HORMONE_H */
