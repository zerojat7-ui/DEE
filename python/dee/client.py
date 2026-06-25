"""
dee.client — DEE API HTTP 클라이언트
Kcode Digital Emotion Engine v1.3.0

DEE C 서버(kc_api_server)와 REST/JSON으로 통신
"""

import json
import urllib.request
import urllib.error
from typing import Optional

from .models import (
    DeeInitResponse,
    DeeChatResponse,
    DeeStatusResponse,
    DeeStimulusResponse,
    DeeVisionResponse,
    DeeBirthResponse,
)

# Provider 상수 (kc_active_ai.h 의 KcAIProvider 와 동일)
PROVIDER_CLAUDE  = 0
PROVIDER_OPENAI  = 1
PROVIDER_GEMINI  = 2
PROVIDER_OLLAMA  = 3
PROVIDER_CUSTOM  = 4

PROVIDER_MAP = {
    "claude":  PROVIDER_CLAUDE,
    "openai":  PROVIDER_OPENAI,
    "gemini":  PROVIDER_GEMINI,
    "ollama":  PROVIDER_OLLAMA,
    "custom":  PROVIDER_CUSTOM,
}


class DeeError(Exception):
    """DEE API 오류"""
    pass


class DeeClient:
    """
    DEE HTTP API 클라이언트

    Args:
        base_url: DEE 서버 주소 (기본: http://localhost:8080)
        timeout:  요청 타임아웃 초 (기본: 30)

    Example:
        dee = DeeClient("http://localhost:8080")
        dee.init(api_key="sk-ant-...", persona="KP01", name="Kcai")
        reply = dee.chat("안녕하세요!")
        print(reply.text)
        print(reply.emotion)
    """

    def __init__(self,
                 base_url: str = "http://localhost:8080",
                 timeout: int = 30):
        self.base_url = base_url.rstrip("/")
        self.timeout  = timeout

    # ── 내부 HTTP 유틸 ──────────────────────────────────────

    def _post(self, path: str, payload: dict) -> dict:
        url  = f"{self.base_url}{path}"
        data = json.dumps(payload).encode("utf-8")
        req  = urllib.request.Request(
            url, data=data,
            headers={"Content-Type": "application/json"},
            method="POST"
        )
        try:
            with urllib.request.urlopen(req, timeout=self.timeout) as resp:
                return json.loads(resp.read().decode("utf-8"))
        except urllib.error.HTTPError as e:
            body = e.read().decode("utf-8")
            raise DeeError(f"HTTP {e.code}: {body}") from e
        except urllib.error.URLError as e:
            raise DeeError(f"서버 연결 실패 ({self.base_url}): {e.reason}") from e

    def _get(self, path: str) -> dict:
        url = f"{self.base_url}{path}"
        req = urllib.request.Request(url, method="GET")
        try:
            with urllib.request.urlopen(req, timeout=self.timeout) as resp:
                return json.loads(resp.read().decode("utf-8"))
        except urllib.error.HTTPError as e:
            body = e.read().decode("utf-8")
            raise DeeError(f"HTTP {e.code}: {body}") from e
        except urllib.error.URLError as e:
            raise DeeError(f"서버 연결 실패 ({self.base_url}): {e.reason}") from e

    # ── 공개 API ────────────────────────────────────────────

    def health(self) -> bool:
        """서버 헬스체크. 정상이면 True"""
        try:
            d = self._get("/dee/health")
            return bool(d.get("ok"))
        except DeeError:
            return False

    def init(self,
             api_key:  str = "",
             provider: str | int = "claude",
             persona:  str = "KP01",
             name:     str = "Kcai") -> DeeInitResponse:
        """
        AI 탄생 또는 재구동.

        Args:
            api_key:  API 키 (Ollama는 빈 문자열 가능)
            provider: "claude" | "openai" | "gemini" | "ollama" | 정수
            persona:  KcPersona 코드 (KP01~KP16) 또는 한글 이름
            name:     AI 이름

        Returns:
            DeeInitResponse
        """
        if isinstance(provider, str):
            provider = PROVIDER_MAP.get(provider.lower(), PROVIDER_CLAUDE)

        d = self._post("/dee/init", {
            "api_key":  api_key,
            "provider": provider,
            "persona":  persona,
            "name":     name,
        })
        return DeeInitResponse.from_dict(d)

    def chat(self, message: str) -> DeeChatResponse:
        """
        대화. 감정 상태가 자동 반영된 응답 반환.

        Args:
            message: 사용자 입력 메시지

        Returns:
            DeeChatResponse (text, emotion, hormone 포함)
        """
        d = self._post("/dee/chat", {"message": message})
        return DeeChatResponse.from_dict(d)

    def status(self) -> DeeStatusResponse:
        """현재 감정·호르몬 상태 조회"""
        d = self._get("/dee/status")
        return DeeStatusResponse.from_dict(d)

    def stimulus(self,
                 event:       str   = "event",
                 goal_impact: float = 0.0,
                 threat:      float = 0.0,
                 fairness:    float = 0.5,
                 certainty:   float = 0.5,
                 social:      float = 0.5) -> DeeStimulusResponse:
        """
        자극 직접 주입.

        Args:
            event:       이벤트 설명 문자열
            goal_impact: 목표 영향 -1.0 ~ 1.0
            threat:      위협 강도 0.0 ~ 1.0
            fairness:    공정성   0.0 ~ 1.0
            certainty:   확실성   0.0 ~ 1.0
            social:      사회성   0.0 ~ 1.0
        """
        d = self._post("/dee/stimulus", {
            "event":       event,
            "goal_impact": goal_impact,
            "threat":      threat,
            "fairness":    fairness,
            "certainty":   certainty,
            "social":      social,
        })
        return DeeStimulusResponse.from_dict(d)

    def vision(self,
               joy:       float = 0.0,
               anger:     float = 0.0,
               fear:      float = 0.0,
               attention: float = 0.5) -> DeeVisionResponse:
        """
        카메라 표정 수치 입력.

        Args:
            joy:       기쁨 표정 0.0 ~ 1.0
            anger:     분노 표정 0.0 ~ 1.0
            fear:      공포 표정 0.0 ~ 1.0
            attention: 시선 집중도 0.0 ~ 1.0
        """
        d = self._post("/dee/vision", {
            "joy":       joy,
            "anger":     anger,
            "fear":      fear,
            "attention": attention,
        })
        return DeeVisionResponse.from_dict(d)

    def birth(self) -> DeeBirthResponse:
        """탄생 기록 조회"""
        d = self._get("/dee/birth")
        return DeeBirthResponse.from_dict(d)

    def save(self) -> bool:
        """현재 상태 저장 (~/.kcode/dee_state.bin). 성공 시 True"""
        d = self._post("/dee/save", {})
        return bool(d.get("ok"))

    # ── 편의 메서드 ─────────────────────────────────────────

    def affinity_up(self, delta: float = 0.1) -> DeeStimulusResponse:
        """호감도 상승 (긍정 자극 주입)"""
        return self.stimulus(
            event="affinity_up",
            goal_impact=delta,
            fairness=0.9,
            social=0.8
        )

    def affinity_down(self, delta: float = 0.1) -> DeeStimulusResponse:
        """호감도 하락 (부정 자극 주입)"""
        return self.stimulus(
            event="affinity_down",
            goal_impact=-delta,
            threat=delta,
            fairness=0.2
        )

    def smile(self, intensity: float = 0.8) -> DeeVisionResponse:
        """미소 입력 단축"""
        return self.vision(joy=intensity, attention=0.9)

    def __repr__(self) -> str:
        return f"DeeClient(base_url={self.base_url!r})"
