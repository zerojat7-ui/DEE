"""
examples/basic_chat.py — DEE Python 클라이언트 기본 예제
Kcode Digital Emotion Engine v1.3.0

실행 전 DEE 서버를 먼저 구동하세요:
    cd DEE-server && make && ./dee-server 8080
"""

import time
import sys
import os

# 패키지 경로 추가 (examples/ 에서 직접 실행할 때)
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from dee import DeeClient

# ── 서버 주소 ────────────────────────────────────────────────
SERVER_URL = "http://localhost:8080"


def print_sep(title: str = ""):
    print("\n" + "─" * 50)
    if title:
        print(f"  {title}")
        print("─" * 50)


def main():
    # ── 1. 클라이언트 생성 ───────────────────────────────────
    dee = DeeClient(SERVER_URL)

    # ── 2. 헬스체크 ─────────────────────────────────────────
    print_sep("DEE 서버 연결 확인")
    if not dee.health():
        print(f"  ❌ 서버에 연결할 수 없습니다: {SERVER_URL}")
        print("  DEE 서버를 먼저 구동하세요:")
        print("    cd DEE-server && make && ./dee-server 8080")
        return

    print(f"  ✅ 서버 연결 성공: {SERVER_URL}")

    # ── 3. AI 탄생 / 재구동 ──────────────────────────────────
    print_sep("AI 초기화")

    # API 키: 환경변수에서 로드 (없으면 오프라인 모드)
    api_key  = os.environ.get("KCODE_CLAUDE_KEY", "")
    provider = "claude" if api_key else "ollama"

    init = dee.init(
        api_key  = api_key,
        provider = provider,
        persona  = "KP01",   # 탐험가 — 활동·직관·공감·탐색
        name     = "Kcai",
    )

    if not init.ok:
        print(f"  ❌ 초기화 실패: {init.error}")
        return

    print(f"  이름:        {init.name}")
    print(f"  탄생일:      {init.born}")
    print(f"  나이:        {init.age}")
    print(f"  성격:        {init.persona}")
    print(f"  구동 횟수:   {init.session_count}회")
    print(f"  지배 감정:   {init.emotion.dominant()}")
    print(f"  도파민:      {init.hormone.dopamine:.3f}")

    # ── 4. 탄생 기록 조회 ────────────────────────────────────
    print_sep("탄생 기록")
    birth = dee.birth()
    print(f"  이름:              {birth.name}")
    print(f"  탄생일:            {birth.born}")
    print(f"  누적 호감도:       {birth.cumulative_affinity:.4f}")

    # ── 5. 대화 ──────────────────────────────────────────────
    print_sep("대화 테스트")

    conversations = [
        "안녕하세요! 오늘 기분이 어때요?",
        "정말 흥미로운 이야기네요! 더 알려주세요.",
        "고마워요, 덕분에 많이 배웠어요.",
    ]

    for msg in conversations:
        print(f"\n  👤 사용자: {msg}")
        reply = dee.chat(msg)

        if not reply.ok:
            print(f"  ❌ 오류: {reply.error}")
            continue

        # 응답 (긴 경우 앞부분만 출력)
        text_preview = reply.text[:120] + "..." if len(reply.text) > 120 else reply.text
        print(f"  🤖 DEE:    {text_preview}")
        print(f"     감정:   {reply.emotion}")
        print(f"     Bio:    {reply.bio:.4f} | tick={reply.tick}")
        time.sleep(0.5)

    # ── 6. 긍정 자극 주입 ────────────────────────────────────
    print_sep("자극 주입 — 긍정 이벤트")
    stim = dee.stimulus(
        event       = "칭찬 받음",
        goal_impact = 0.8,
        fairness    = 0.9,
        social      = 0.7,
    )
    print(f"  자극 후 감정: {stim.emotion}")
    print(f"  도파민: {stim.hormone.dopamine:.3f}  옥시토신: {stim.hormone.oxytocin:.3f}")

    # ── 7. 카메라(미소) 입력 ─────────────────────────────────
    print_sep("비전 입력 — 미소")
    vis = dee.smile(intensity=0.9)
    print(f"  비전 후 감정: {vis.emotion}")
    print(f"  도파민: {vis.hormone.dopamine:.3f}  옥시토신: {vis.hormone.oxytocin:.3f}")

    # ── 8. 최종 상태 ─────────────────────────────────────────
    print_sep("최종 감정·호르몬 상태")
    status = dee.status()
    print(f"  tick:       {status.tick}")
    print(f"  Bio:        {status.bio:.4f}")
    print(f"  지배 감정:  {status.emotion.dominant()}")
    print(f"  호감도:     {status.emotion.affinity:.4f}")
    print(f"  피로:       {status.emotion.fatigue:.4f}")
    print()
    print("  【감정 전체】")
    e = status.emotion
    emo_fields = ["anger","fear","joy","sadness",
                  "curiosity","disgust","surprise","trust"]
    for f in emo_fields:
        val  = getattr(e, f)
        bars = int(val * 20)
        bar  = "█" * bars + "░" * (20 - bars)
        print(f"    {f:12s} {val:.3f}  {bar}")

    # ── 9. 상태 저장 ─────────────────────────────────────────
    print_sep("상태 저장")
    ok = dee.save()
    print(f"  {'✅ 저장 완료' if ok else '❌ 저장 실패'}")

    print("\n" + "─" * 50)
    print("  예제 완료.")
    print("─" * 50 + "\n")


if __name__ == "__main__":
    main()
