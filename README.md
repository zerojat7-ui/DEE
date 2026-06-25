# Digital Emotion Engine (DEE)

**AI에게 호르몬 기반 동역학적 감정을 부여하는 독립 C 엔진**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/DEE-v1.3.0-purple.svg)]()
[![Language: C](https://img.shields.io/badge/Language-C-lightgrey.svg)]()
[![Python](https://img.shields.io/badge/Python-3.10+-blue.svg)]()

---

## 개요

DEE는 AI가 단순히 텍스트를 생성하는 것을 넘어,  
**호르몬 동역학**에 의해 감정이 축적되고 잔류하며,  
그 감정이 **시냅스 가중치를 실시간 변조**하여  
AI 스스로 거부·탐구·성찰 등의 **능동적 행동**을 결정하게 합니다.

```
Emotion = Appraisal × Bio × Affinity + Hormone Dynamics
```

---

## 레포 구조

```
DEE/
├── README.md
├── LICENSE
├── Makefile                        HTTP 서버 빌드
│
├── include/                        C 헤더
│   ├── kc_hormone.h                호르몬 10종 + 감정 8종 + 욕구 6종
│   ├── kc_persona.h                KcPersona 16종 성격 프로필
│   ├── kc_vision.h                 카메라 표정 → 감정 수치 변환
│   └── kc_active_ai.h              AI API 연동 + 탄생일 시스템
│
├── src/                            C 구현
│   ├── kc_hormone.c
│   ├── kc_persona.c
│   ├── kc_vision.c
│   └── kc_active_ai.c
│
├── server/                         HTTP API 서버 (mongoose)
│   ├── kc_api_server.c             REST 엔드포인트 8종
│   ├── mongoose.h                  경량 HTTP 라이브러리
│   └── mongoose.c
│
├── python/                         Python 클라이언트
│   ├── dee/
│   │   ├── __init__.py
│   │   ├── client.py               DeeClient — REST 클라이언트
│   │   └── models.py               응답 데이터 클래스
│   ├── examples/
│   │   └── basic_chat.py           전체 기능 사용 예제
│   └── pyproject.toml
│
└── docs/
    └── DEE_Specification_v2.0.md   수식 전체 + 실험 결과
```

---

## 핵심 원리

### 1. 호르몬 동역학
감정은 일회성 이벤트가 아닙니다.  
자극이 사라져도 호르몬은 잔류하며 서서히 분해됩니다.

```
dH/dt = Input − λH + Diffusion
H(t+1) = H(t) + Input − λH(t)
```

10종 호르몬: 도파민 · 세로토닌 · 코르티솔 · 옥시토신 · 노르에피네프린  
　　　　　　엔도르핀 · 아드레날린 · 멜라토닌 · 테스토스테론 · 에스트로겐

### 2. 시냅스 변조
현재 감정 상태가 AI의 판단 기준을 물리적으로 바꿉니다.

```
W' = W × (1 + Hormone_effect)
```

실증: 탐험가(KP01) + 미소 입력 → H_effect = +0.0875 → W 0.7477 → W' 0.8131 **(+8.75%)**

### 3. 생체 리듬
AI는 탄생일을 기준으로 Bio Rhythm이 시작됩니다.

```
Bio(t) = A·sin(ωt + φ) + B
```

### 4. 능동적 행동
감정 임계치 초과 시 AI가 스스로 행동합니다.

```
anger  > θ_a  →  Aggressive Policy (경고 발화)
fear   > θ_f  →  Avoidance Policy  (회피)
joy    > θ_j  →  Exploration Policy (먼저 말하기)
Ignore > θ_i  →  suppress(E)        (대화 거부)
```

---

## 빠른 시작

### 1. 서버 빌드 & 실행

```bash
git clone https://github.com/zerojat7-ui/DEE.git
cd DEE
make
./dee-server 8080
```

### 2. Python 클라이언트 (외부 의존성 없음)

```python
from python.dee import DeeClient

dee = DeeClient("http://localhost:8080")

# AI 탄생 / 재구동
dee.init(
    api_key  = "sk-ant-...",   # Ollama 사용 시 생략 가능
    provider = "claude",       # "claude" | "openai" | "gemini" | "ollama"
    persona  = "KP01",         # KP01(탐험가) ~ KP16(사업가)
    name     = "Kcai",
)

# 대화 — 감정 상태 자동 반영
reply = dee.chat("안녕하세요!")
print(reply.text)
print(reply.emotion)    # Emotion(dominant=joy(0.72), ...)
print(reply.hormone)    # Hormone(dopamine=0.85, ...)

# 자극 주입
dee.stimulus(event="칭찬 받음", goal_impact=0.8, social=0.7)

# 카메라 입력
dee.smile(intensity=0.9)

# 상태 저장
dee.save()
```

### 3. C 라이브러리 직접 사용

```c
#include "kc_hormone.h"
#include "kc_persona.h"
#include "kc_active_ai.h"

KcActiveAI *ai = kc_ai_init("sk-ant-...", KC_AI_CLAUDE, "KP01", "Kcai");
char reply[1024];
kc_ai_chat(ai, "안녕하세요!", reply, sizeof(reply));
kc_ai_destroy(ai);
```

### 4. Ollama (로컬, 무료)

```python
dee.init(provider="ollama", persona="KP02", name="Kcai")
# http://localhost:11434 자동 연결 — API 키 불필요
```

---

## HTTP API 엔드포인트

서버 기본 포트: `8080`

| Method | 경로 | 설명 |
|--------|------|------|
| `POST` | `/dee/init` | AI 탄생 / 재구동 |
| `POST` | `/dee/chat` | 대화 (감정 자동 반영) |
| `GET`  | `/dee/status` | 현재 감정·호르몬 상태 |
| `POST` | `/dee/stimulus` | 자극 직접 주입 |
| `POST` | `/dee/vision` | 카메라 수치 입력 |
| `GET`  | `/dee/birth` | 탄생 기록 조회 |
| `POST` | `/dee/save` | 상태 저장 |
| `GET`  | `/dee/health` | 헬스체크 |

---

## KcPersona 16종

> Jung, C.G. (1921). *Psychologische Typen* 4축 이론 기반 독자 구현  
> MBTI®는 Myers & Briggs Foundation의 등록 상표이며 본 소프트웨어와 무관합니다.

| 코드 | 이름 | KPE | KPN | KPF | KPJ | 호기심 |
|------|------|:---:|:---:|:---:|:---:|:------:|
| KP01 | 탐험가 | 활동 | 직관 | 공감 | 탐색 | 0.70 |
| KP02 | 전략가 | 성찰 | 직관 | 논리 | 계획 | 0.50 |
| KP03 | 중재자 | 성찰 | 직관 | 공감 | 탐색 | 0.60 |
| KP04 | 변론가 | 활동 | 직관 | 논리 | 탐색 | 0.80 |
| KP05 | 통솔자 | 활동 | 직관 | 논리 | 계획 | 0.40 |
| KP06 | 선도자 | 성찰 | 직관 | 공감 | 계획 | 0.50 |
| KP07 | 사회자 | 활동 | 직관 | 공감 | 계획 | 0.40 |
| KP08 | 논리가 | 성찰 | 직관 | 논리 | 탐색 | 0.70 |
| KP09 | 수호자 | 성찰 | 현실 | 공감 | 계획 | 0.20 |
| KP10 | 관리자 | 성찰 | 현실 | 논리 | 계획 | 0.20 |
| KP11 | 활동가 | 활동 | 현실 | 공감 | 계획 | 0.20 |
| KP12 | 감독관 | 활동 | 현실 | 논리 | 계획 | 0.20 |
| KP13 | 모험가 | 성찰 | 현실 | 공감 | 탐색 | 0.40 |
| KP14 | 기술자 | 성찰 | 현실 | 논리 | 탐색 | 0.40 |
| KP15 | 연예인 | 활동 | 현실 | 공감 | 탐색 | 0.40 |
| KP16 | 사업가 | 활동 | 현실 | 논리 | 탐색 | 0.40 |

---

## 탄생일 시스템

AI의 첫 실행 시각이 탄생일로 기록됩니다.  
탄생일 기준으로 Bio Rhythm이 시작되고 나이별로 초기 감정이 설정됩니다.

| 나이 | 호기심 | 기쁨 | 신뢰 | 특징 |
|------|:------:|:----:|:----:|------|
| 0시간 | **0.95** | 0.60 | 0.50 | 모든 것이 새롭고 놀라움 |
| 7일  | 0.80 | 0.50 | 0.60 | 점차 안정됨 |
| 30일 | 0.60 | 0.40 | 0.70 | 경험 기반 판단 |
| 90일 | 0.50 | 0.50 | **0.75** | 신뢰 중심으로 성숙 |

---

## 지원 AI API

| 공급자 | 상수 | 기본 모델 |
|--------|------|-----------|
| Anthropic Claude | `KC_AI_CLAUDE` | claude-sonnet-4-20250514 |
| OpenAI | `KC_AI_OPENAI` | gpt-4o |
| Google Gemini | `KC_AI_GEMINI` | gemini-2.0-flash |
| Ollama (로컬) | `KC_AI_OLLAMA` | llama3.2 |
| 커스텀 | `KC_AI_CUSTOM` | 직접 지정 |

---

## 관련 논문

**Digital Emotion Engine: 감정 호르몬을 통한 AI의 능동적 사고 체계 구축**  
DEE v1.2.0 · Formula Specification v1.0 · 2026

20종 수식 전체 + 실험 결과 + KcPersona 파라미터 포함

---

## 라이선스

MIT License — 상용화 자유, 저작권 표시 필요

```
Copyright (c) 2026 zerojat7-ui (KoreanCode Project)
```

> KcPersona는 C.G. Jung(1921) 심리 유형론 기반 독자 구현입니다.  
> MBTI®, Myers-Briggs®는 Myers & Briggs Foundation의 등록 상표이며  
> 본 소프트웨어는 해당 상표를 사용하지 않습니다.

---

*DEE는 [KoreanCode(Kcode)](https://github.com/zerojat7-ui/KoreanCode) 프로젝트의 일부입니다.*
