# ════════════════════════════════════════════════════════
# Makefile — DEE (Digital Emotion Engine)
# Kcode DEE v1.3.0
#
# 레포 구조 (B형):
#   DEE/
#   ├── Makefile          ← 여기
#   ├── include/          ← 헤더
#   ├── src/              ← C 소스
#   ├── server/           ← HTTP 서버 (mongoose)
#   ├── python/           ← Python 클라이언트
#   └── build/            ← 빌드 결과물 (gitignore)
# ════════════════════════════════════════════════════════

CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -Iinclude -Iserver
LIBS    = -lm

BUILD   = build
TARGET  = $(BUILD)/dee-server

MONGOOSE_H  = server/mongoose.h
MONGOOSE_C  = server/mongoose.c
MONGOOSE_URL_H = https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.h
MONGOOSE_URL_C = https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.c

SRCS    = server/kc_api_server.c \
          server/mongoose.c \
          src/kc_hormone.c \
          src/kc_persona.c \
          src/kc_vision.c \
          src/kc_active_ai.c

# ── build 디렉토리 생성 ──────────────────────────────────
$(BUILD):
	mkdir -p $(BUILD)

# ── mongoose 다운로드 ────────────────────────────────────
$(MONGOOSE_H):
	@echo "  📥 mongoose.h 다운로드 중..."
	@curl -sL $(MONGOOSE_URL_H) -o $(MONGOOSE_H)
	@echo "  ✅ mongoose.h 완료"

$(MONGOOSE_C):
	@echo "  📥 mongoose.c 다운로드 중..."
	@curl -sL $(MONGOOSE_URL_C) -o $(MONGOOSE_C)
	@echo "  ✅ mongoose.c 완료"

# ── 의존성 설치 ─────────────────────────────────────────
setup: $(MONGOOSE_H) $(MONGOOSE_C)
	@echo ""
	@echo "  ✅ 의존성 설치 완료. 이제 make 로 빌드하세요."
	@echo ""

# ── 기본 빌드 ───────────────────────────────────────────
all: $(MONGOOSE_H) $(MONGOOSE_C) $(BUILD) $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)
	@echo ""
	@echo "  ✅ 빌드 완료: ./$(TARGET)"
	@echo "  실행: ./$(TARGET) [포트번호]  (기본: 8080)"
	@echo ""

# ── 디버그 빌드 ─────────────────────────────────────────
debug: CFLAGS += -g -DDEBUG -fsanitize=address
debug: $(MONGOOSE_H) $(MONGOOSE_C) $(BUILD) $(TARGET)

# ── 정리 ────────────────────────────────────────────────
clean:
	rm -rf $(BUILD)

# mongoose 포함 전체 정리
distclean: clean
	rm -f $(MONGOOSE_H) $(MONGOOSE_C)

# ── 실행 (기본 8080) ────────────────────────────────────
run: all
	./$(TARGET) 8080

.PHONY: all setup debug clean distclean run
