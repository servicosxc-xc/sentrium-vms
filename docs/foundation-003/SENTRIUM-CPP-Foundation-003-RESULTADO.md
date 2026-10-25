# SENTRIUM-CPP-Foundation-003 — RESULTADO

**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-003 — Device Foundation  
**Branch:** `foundation/003-device-foundation`  
**Estado técnico:** **PASS / READY FOR HUMAN REVIEW**  
**Data:** 25/10/2026

---

## 1. Resultado executivo

A Foundation-003 atingiu seu objetivo técnico: introduzir a primeira base real de dispositivos do SENTRIUM VMS e validar uma integração Intelbras/Dahua NetSDK nativa C++ por IP/LAN, sem iniciar ainda a pipeline de vídeo.

Resultado:

**FOUNDATION-003 — PASS / READY FOR HUMAN REVIEW**

Este estado não representa aprovação humana e não promove automaticamente ADR-010.

---

## 2. Entrega implementada

Foi introduzido:

- modelo C++ de dispositivo independente de Qt;
- estados de conexão;
- canais derivados do resultado real de login;
- adapter `sentrium-intelbras`;
- uso do header oficial `dhnetsdk.h`;
- carregamento dinâmico do `dhnetsdk.dll` externo;
- subset NetSDK:
  - `CLIENT_InitEx`;
  - `CLIENT_SetAutoReconnect`;
  - `CLIENT_LoginEx2`;
  - `CLIENT_GetLastError`;
  - `CLIENT_Logout`;
  - `CLIENT_Cleanup`;
- worker dedicado para operações de dispositivo;
- controller Qt/QML;
- formulário Adicionar dispositivo;
- conectar;
- desconectar;
- remover;
- árvore real de dispositivo/canais;
- credenciais mantidas apenas em memória;
- senha ausente do modelo QML e de evidências.

Nenhum componente de vídeo foi introduzido.

---

## 3. SDK Intelbras externo

O SDK permaneceu fora do repositório.

Root utilizado pelo build:

`SENTRIUM_INTELBRAS_SDK_ROOT`

O PRE-FLIGHT validou:

- `include/dhnetsdk.h`;
- `bin/dhnetsdk.dll`;
- `bin/dhnetsdk.lib`;
- arquitetura x64 do runtime;
- coexistência com toolchain e Qt aprovados.

O build compilou o adapter diretamente contra o header oficial do kit externo.

Nenhuma incompatibilidade de assinatura usada nesta Foundation foi detectada pelo MSVC.

Isso não constitui validação completa de toda a ABI do SDK nem autorização de redistribuição.

---

## 4. Build e testes

### Debug

- configure: PASS;
- PRE-FLIGHT F003: PASS;
- build: PASS;
- `sentrium-client.exe`: PASS;
- testes: **27/27 PASS**.

Composição:

- 8 unit tests da base;
- 11 integration tests da base;
- 7 Device Foundation tests;
- 1 smoke gráfico.

### Release

- configure: PASS;
- PRE-FLIGHT F003: PASS;
- build: PASS;
- `sentrium-client.exe`: PASS;
- testes: **27/27 PASS**;
- processo residual após suíte: **0**.

---

## 5. Validação com equipamento real

Foi executado teste real com equipamento Intelbras acessível em LAN.

Caminho:

**SENTRIUM VMS → NetSDK nativo C++ → equipamento Intelbras por IP/LAN**

Resultado observado:

- login: PASS;
- estado Connected: PASS;
- quantidade real de canais obtida: 18;
- árvore populada com Canal 01 até Canal 18;
- disconnect: PASS;
- reconnect: PASS;
- múltiplos ciclos disconnect/connect: PASS;
- fechamento normal após ciclos: PASS.

Cloud/P2P não participou deste teste.

---

## 6. Segurança de credenciais

Nesta Foundation:

- usuário/senha são fornecidos pelo operador;
- senha permanece somente em memória;
- senha não é persistida;
- senha não entra no modelo exposto ao QML;
- senha não deve aparecer em logs/evidências;
- nenhum banco de credenciais foi criado.

Persistência segura continua fora de escopo.

---

## 7. Limites preservados

Não foram introduzidos:

- PlaySDK;
- `CLIENT_RealPlayEx`;
- RAW/DHAV callbacks;
- FFmpeg;
- decode;
- vídeo;
- mosaico funcional;
- gravação;
- playback;
- snapshot;
- PTZ;
- eventos de câmera;
- IA;
- LPR;
- libt2u;
- Cloud/P2P;
- Relay;
- HTTP;
- WebSocket;
- IPC;
- SQLite;
- persistência de dispositivo;
- persistência de credenciais;
- RBAC;
- plugin framework;
- event bus genérico;
- command bus genérico.

---

## 8. Interface

A interface foi evoluída apenas o necessário para consumir dispositivos reais.

O visual continua **provisório**.

A aprovação técnica da F003 não congela:

- layout;
- árvore visual;
- botões;
- tipografia;
- cores;
- UX definitiva.

O redesign deverá acompanhar Live View e mosaico reais.

---

## 9. ADRs

- ADR-004 — client-framework: permanece Proposed.
- ADR-010 — interoperability-intelbras: permanece Proposed.

A F003 fornece evidência positiva de interoperabilidade NetSDK por LAN, mas não cobre:

- Cloud/P2P;
- redistribuição;
- matriz ampla de equipamentos/firmwares;
- RealPlay;
- PlaySDK;
- recovery de mídia.

Portanto não há promoção automática de ADR.

---

## 10. Critérios de aceite

- F003-AC-01 — Debug x64: PASS.
- F003-AC-02 — Release x64: PASS.
- F003-AC-03 — regressão F001/F002: PASS.
- F003-AC-04 — NetSDK externo sem entrar no Git: PASS.
- F003-AC-05 — core público sem Qt/NetSDK: PASS.
- F003-AC-06 — runtime NetSDK com ownership/cleanup determinístico: PASS para o escopo testado.
- F003-AC-07 — senha não exposta em modelo/log/evidência: PASS para os testes realizados.
- F003-AC-08 — adicionar dispositivo em memória: PASS.
- F003-AC-09 — conectar/desconectar: PASS.
- F003-AC-10 — canais derivados do login: PASS.
- F003-AC-11 — árvore real dispositivo/canais: PASS.
- F003-AC-12 — diagnóstico sanitizado de falha: PASS nos testes automatizados.
- F003-AC-13 — shutdown sem sessão ativa no fluxo testado: PASS.
- F003-AC-14 — equipamento Intelbras real por LAN: PASS.
- F003-AC-15 — reconnect controlado: PASS.
- F003-AC-16 — zero processo residual: PASS.
- F003-AC-17 — sem Cloud/P2P/PlaySDK/vídeo: PASS.
- F003-AC-18 — sem alegação de produção/redistribuição: PASS.

---

## 11. Estado técnico final

- PRE-FLIGHT: PASS.
- Debug build: PASS.
- Debug tests: 27/27 PASS.
- Release build: PASS.
- Release tests: 27/27 PASS.
- Intelbras LAN real: PASS.
- 18 canais reais enumerados.
- Disconnect/reconnect: PASS.
- Processo residual: 0.
- Escopo proibido: não introduzido.
- ADR-004: Proposed.
- ADR-010: Proposed.
- Merge em `main`: não realizado.
- Aprovação humana: pendente.

# FOUNDATION-003 — PASS / READY FOR HUMAN REVIEW
