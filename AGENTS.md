# Codex Rules

이 저장소에서는 Codex가 아래 작업을 하지 않는다.

- UnrealBuildTool, RunUAT, msbuild, dotnet build, dotnet test 실행
- UnrealEditor, Rider, 게임 실행, 자동 테스트 실행
- 빌드 결과 확인을 위한 컴파일/리빌드/클린 실행

허용되는 작업은 다음만 한다.

- 코드 읽기
- 로그 읽기
- `git diff` 확인
- 정적 분석 기반 수정

빌드나 컴파일 확인이 필요하면 먼저 사용자에게 확인을 요청한다.

[//]: # (# Project Planning Docs)

[//]: # ()
[//]: # (Codex는 코드 수정 전에 아래 기획서를 기본 기준으로 삼는다.)

[//]: # ()
[//]: # (- `Docs/Bench.md`: 볼 대기석 시스템 기획)

[//]: # (- `Docs/DeploymentSlot.md`: 볼 출전 슬롯 시스템 기획)

[//]: # ()
[//]: # (관련 기능을 수정할 때는 먼저 해당 문서를 읽고, 문서 내용과 맞지 않는 변경은 하지 않는다.)

[//]: # (문서가 모호하거나 코드와 충돌하면 사용자에게 확인한다.)