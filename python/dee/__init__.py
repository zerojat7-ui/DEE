"""
dee — DEE API Python 클라이언트
Kcode Digital Emotion Engine v1.3.0

사용 예:
    from dee import DeeClient

    dee = DeeClient("http://localhost:8080")
    dee.init(api_key="sk-ant-...", provider="claude", persona="KP01", name="Kcai")

    reply = dee.chat("안녕하세요!")
    print(reply.text)
    print(reply.emotion)
    print(reply.hormone)
"""

from .client import DeeClient
from .models import (
    DeeInitResponse,
    DeeChatResponse,
    DeeStatusResponse,
    DeeStimulusResponse,
    DeeVisionResponse,
    DeeBirthResponse,
)

__all__ = [
    "DeeClient",
    "DeeInitResponse",
    "DeeChatResponse",
    "DeeStatusResponse",
    "DeeStimulusResponse",
    "DeeVisionResponse",
    "DeeBirthResponse",
]

__version__ = "1.3.0"
