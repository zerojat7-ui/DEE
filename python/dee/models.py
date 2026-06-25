"""
dee.models — DEE API 응답 데이터 클래스
Kcode Digital Emotion Engine v1.3.0
"""

from dataclasses import dataclass, field
from typing import Optional


# ── 감정 상태 ────────────────────────────────────────────────
@dataclass
class Emotion:
    anger:      float = 0.0
    fear:       float = 0.0
    joy:        float = 0.0
    sadness:    float = 0.0
    curiosity:  float = 0.0
    disgust:    float = 0.0
    surprise:   float = 0.0
    trust:      float = 0.0
    fatigue:    float = 0.0
    irritation: float = 0.0
    affinity:   float = 0.0
    intensity:  float = 0.0

    @classmethod
    def from_dict(cls, d: dict) -> "Emotion":
        return cls(**{k: float(v) for k, v in d.items() if k in cls.__dataclass_fields__})

    def dominant(self) -> str:
        """지배 감정 이름 반환"""
        fields = ["anger","fear","joy","sadness",
                  "curiosity","disgust","surprise","trust"]
        return max(fields, key=lambda f: getattr(self, f))

    def __str__(self) -> str:
        dom = self.dominant()
        val = getattr(self, dom)
        return (f"Emotion(dominant={dom}({val:.3f}), "
                f"joy={self.joy:.3f}, trust={self.trust:.3f}, "
                f"curiosity={self.curiosity:.3f}, affinity={self.affinity:.3f})")


# ── 호르몬 상태 ──────────────────────────────────────────────
@dataclass
class Hormone:
    dopamine:       float = 0.0
    serotonin:      float = 0.0
    cortisol:       float = 0.0
    oxytocin:       float = 0.0
    norepinephrine: float = 0.0
    endorphin:      float = 0.0
    adrenaline:     float = 0.0
    melatonin:      float = 0.0
    testosterone:   float = 0.0
    estrogen:       float = 0.0

    @classmethod
    def from_dict(cls, d: dict) -> "Hormone":
        return cls(**{k: float(v) for k, v in d.items() if k in cls.__dataclass_fields__})

    def __str__(self) -> str:
        return (f"Hormone(dopamine={self.dopamine:.3f}, "
                f"serotonin={self.serotonin:.3f}, "
                f"cortisol={self.cortisol:.3f}, "
                f"oxytocin={self.oxytocin:.3f})")


# ── 응답 클래스들 ────────────────────────────────────────────
@dataclass
class DeeInitResponse:
    ok:            bool
    name:          str        = ""
    persona:       str        = ""
    born:          str        = ""
    age:           str        = ""
    session_count: int        = 0
    emotion:       Emotion    = field(default_factory=Emotion)
    hormone:       Hormone    = field(default_factory=Hormone)
    error:         Optional[str] = None

    @classmethod
    def from_dict(cls, d: dict) -> "DeeInitResponse":
        return cls(
            ok            = bool(d.get("ok", False)),
            name          = d.get("name", ""),
            persona       = d.get("persona", ""),
            born          = d.get("born", ""),
            age           = d.get("age", ""),
            session_count = int(d.get("session_count", 0)),
            emotion       = Emotion.from_dict(d["emotion"]) if "emotion" in d else Emotion(),
            hormone       = Hormone.from_dict(d["hormone"]) if "hormone" in d else Hormone(),
            error         = d.get("error"),
        )


@dataclass
class DeeChatResponse:
    ok:      bool
    text:    str        = ""
    tick:    int        = 0
    bio:     float      = 0.0
    emotion: Emotion    = field(default_factory=Emotion)
    hormone: Hormone    = field(default_factory=Hormone)
    error:   Optional[str] = None

    @classmethod
    def from_dict(cls, d: dict) -> "DeeChatResponse":
        return cls(
            ok      = bool(d.get("ok", False)),
            text    = d.get("reply", ""),
            tick    = int(d.get("tick", 0)),
            bio     = float(d.get("bio", 0.0)),
            emotion = Emotion.from_dict(d["emotion"]) if "emotion" in d else Emotion(),
            hormone = Hormone.from_dict(d["hormone"]) if "hormone" in d else Hormone(),
            error   = d.get("error"),
        )


@dataclass
class DeeStatusResponse:
    ok:            bool
    name:          str        = ""
    persona:       str        = ""
    age:           str        = ""
    tick:          int        = 0
    bio:           float      = 0.0
    session_count: int        = 0
    msg_count:     int        = 0
    emotion:       Emotion    = field(default_factory=Emotion)
    hormone:       Hormone    = field(default_factory=Hormone)
    error:         Optional[str] = None

    @classmethod
    def from_dict(cls, d: dict) -> "DeeStatusResponse":
        return cls(
            ok            = bool(d.get("ok", False)),
            name          = d.get("name", ""),
            persona       = d.get("persona", ""),
            age           = d.get("age", ""),
            tick          = int(d.get("tick", 0)),
            bio           = float(d.get("bio", 0.0)),
            session_count = int(d.get("session_count", 0)),
            msg_count     = int(d.get("msg_count", 0)),
            emotion       = Emotion.from_dict(d["emotion"]) if "emotion" in d else Emotion(),
            hormone       = Hormone.from_dict(d["hormone"]) if "hormone" in d else Hormone(),
            error         = d.get("error"),
        )


@dataclass
class DeeStimulusResponse:
    ok:      bool
    tick:    int        = 0
    emotion: Emotion    = field(default_factory=Emotion)
    hormone: Hormone    = field(default_factory=Hormone)
    error:   Optional[str] = None

    @classmethod
    def from_dict(cls, d: dict) -> "DeeStimulusResponse":
        return cls(
            ok      = bool(d.get("ok", False)),
            tick    = int(d.get("tick", 0)),
            emotion = Emotion.from_dict(d["emotion"]) if "emotion" in d else Emotion(),
            hormone = Hormone.from_dict(d["hormone"]) if "hormone" in d else Hormone(),
            error   = d.get("error"),
        )


@dataclass
class DeeVisionResponse:
    ok:      bool
    tick:    int        = 0
    emotion: Emotion    = field(default_factory=Emotion)
    hormone: Hormone    = field(default_factory=Hormone)
    error:   Optional[str] = None

    @classmethod
    def from_dict(cls, d: dict) -> "DeeVisionResponse":
        return cls(
            ok      = bool(d.get("ok", False)),
            tick    = int(d.get("tick", 0)),
            emotion = Emotion.from_dict(d["emotion"]) if "emotion" in d else Emotion(),
            hormone = Hormone.from_dict(d["hormone"]) if "hormone" in d else Hormone(),
            error   = d.get("error"),
        )


@dataclass
class DeeBirthResponse:
    ok:                  bool
    name:                str   = ""
    born:                str   = ""
    persona:             str   = ""
    age:                 str   = ""
    session_count:       int   = 0
    cumulative_affinity: float = 0.0
    first_words:         str   = ""
    error:               Optional[str] = None

    @classmethod
    def from_dict(cls, d: dict) -> "DeeBirthResponse":
        return cls(
            ok                  = bool(d.get("ok", False)),
            name                = d.get("name", ""),
            born                = d.get("born", ""),
            persona             = d.get("persona", ""),
            age                 = d.get("age", ""),
            session_count       = int(d.get("session_count", 0)),
            cumulative_affinity = float(d.get("cumulative_affinity", 0.0)),
            first_words         = d.get("first_words", ""),
            error               = d.get("error"),
        )
