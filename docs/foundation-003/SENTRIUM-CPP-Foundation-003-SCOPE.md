# SENTRIUM-CPP-Foundation-003-SCOPE 0.1 — DEVICE FOUNDATION

**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-003  
**Branch:** `foundation/003-device-foundation`  
**Estado:** **PROPOSED — NOT YET AUTHORIZED FOR IMPLEMENTATION**  
**Princípio:** arquitetura suficiente para o problema real.

---

## 1. Objetivo

Introduzir a primeira base real de dispositivos do SENTRIUM VMS e provar uma integração Intelbras/Dahua NetSDK nativa C++ por **IP/LAN**, sem ainda iniciar a pipeline de vídeo.

Ao final desta Foundation, o operador deverá conseguir:

1. abrir o SENTRIUM;
2. adicionar um dispositivo Intelbras por IP/porta/usuário/senha;
3. conectar via NetSDK;
4. visualizar o dispositivo real na árvore;
5. visualizar os canais enumerados pelo login;
6. desconectar/remover o dispositivo;
7. encerrar a aplicação sem recursos SDK residuais.

Nenhum vídeo será exibido nesta Foundation.

---

## 2. Base técnica

A implementação deverá partir da `main` após o merge aprovado da Foundation-002.

Baseline preservada:

- C++20.
- MSVC/v143.
- x64.
- CMake 4.4.4.
- Ninja 1.13.2.
- Qt 6.12.0 para o cliente.
- Core existente permanece independente de Qt.

SDK externo identificado pela auditoria:

`General_NetSDK_3.050_PlaySDK_3.042`

Artefatos NetSDK esperados no kit externo:

- `dhnetsdk.h`
- `dhnetsdk.lib`
- `dhconfigsdk.lib`
- DLLs correspondentes do kit coeso.

O SDK não será copiado para o repositório nem tratado como redistribuível.

---

## 3. Escopo funcional

### 3.1. Modelo de dispositivo

Criar modelo C++ independente de Qt contendo apenas o necessário para esta Foundation:

- DeviceId interno.
- nome amigável;
- host/IP;
- porta;
- estado de conexão;
- tipo/fabricante;
- quantidade de canais;
- lista de canais;
- erro/diagnóstico atual sanitizado.

Não criar capability framework genérico.

### 3.2. Estados

Estados mínimos:

- Disconnected.
- Connecting.
- Connected.
- Disconnecting.
- Failed.

Não usar estado P2P Direct/Relay nesta Foundation, pois a conexão autorizada é IP/LAN.

A semântica futura permanece:

- azul = P2P Direct;
- amarelo = Relay.

Essas cores não devem ser reutilizadas de forma enganosa para LAN.

### 3.3. Intelbras NetSDK

Criar adaptador isolado, por exemplo:

- `sentrium-intelbras`

Ele deverá encapsular apenas o subset necessário da F003.

Subset inicial esperado:

- `CLIENT_InitEx`
- `CLIENT_SetAutoReconnect`
- `CLIENT_LoginEx2`
- `CLIENT_GetLastError`
- `CLIENT_Logout`
- `CLIENT_Cleanup`

Não portar o wrapper C# inteiro.

Callbacks/lifetimes devem permanecer válidos até shutdown completo do runtime.

### 3.4. Runtime NetSDK

O runtime NetSDK deve ter ownership explícito e RAII.

Requisitos:

- inicialização uma vez por processo;
- cleanup determinístico;
- callbacks com lifetime controlado;
- operações de login/logout serializadas inicialmente em um worker dedicado, até existir evidência para política mais concorrente;
- shutdown sem callback usando objeto destruído;
- sem descarregar DLL enquanto houver sessão/callback ativo.

### 3.5. Canais

Para esta Foundation:

- quantidade de canais deve vir do resultado real de login;
- índices internos podem ser zero-based;
- apresentação inicial pode usar `Canal 01`, `Canal 02`, etc.;
- não inventar nomes reais do equipamento;
- não declarar capabilities não consultadas.

### 3.6. Interface

Evoluir o shell atual apenas o suficiente para consumir a funcionalidade real.

Adicionar:

- ação `Adicionar dispositivo`;
- formulário simples com:
  - nome;
  - IP/host;
  - porta;
  - usuário;
  - senha;
- conectar;
- desconectar;
- remover;
- árvore de dispositivo/canais real.

A UI atual continua provisória. Não fazer redesign amplo nesta Foundation.

### 3.7. Credenciais

Nesta Foundation:

- credenciais ficam somente em memória;
- senha não é persistida;
- senha não aparece em logs;
- senha não entra em BuildId/evidências;
- nenhum banco de credenciais será criado.

Persistência segura será tratada quando houver consumidor real para isso.

---

## 4. Dependência externa do SDK

O build deverá localizar o SDK por variável explícita, por exemplo:

`SENTRIUM_INTELBRAS_SDK_ROOT`

Regras:

- não hardcodear caminho pessoal no CMake;
- não baixar SDK automaticamente;
- não modificar vcpkg para obter NetSDK;
- não copiar binários proprietários para Git;
- preflight deve validar headers/libs/DLLs necessários;
- falha na presença/arquitetura do kit = BLOCKED, não fallback silencioso.

---

## 5. Fora do escopo

Explicitamente proibido nesta Foundation:

- PlaySDK.
- RealPlay.
- `CLIENT_RealPlayEx`.
- callbacks RAW/DHAV.
- FFmpeg.
- decode.
- render de vídeo.
- mosaico funcional de vídeo.
- gravação.
- playback.
- snapshot.
- PTZ.
- eventos de câmera.
- IA.
- LPR.
- libt2u.
- Cloud/P2P.
- Relay.
- HTTP.
- WebSocket.
- IPC.
- SQLite.
- persistência de dispositivos.
- persistência de credenciais.
- RBAC.
- plugin framework.
- event bus/command bus genéricos.

---

## 6. Testes

### 6.1. Automatizados sem hardware

Testar:

- transições de estado;
- validação de host/porta;
- modelo de canais;
- add/remove/disconnect;
- senha não exposta em diagnóstico/log;
- shutdown de worker/runtime;
- comportamento de adapter fake;
- UI consumindo fake realista, sem câmera fictícia aparecendo por padrão;
- regressão dos 20 testes F002.

### 6.2. Integração real autorizada

Antes de PASS final, executar em pelo menos um equipamento Intelbras compatível disponível no ambiente do responsável:

- init NetSDK;
- login por IP/LAN;
- confirmação de sucesso;
- obtenção da quantidade real de canais;
- árvore mostrando a quantidade correspondente;
- logout;
- reconnect controlado;
- fechamento do cliente;
- zero processo residual.

Credenciais do equipamento não devem ser enviadas ao repositório ou registradas em evidências.

---

## 7. Critérios de aceite

**F003-AC-01** — build Debug x64 PASS.  
**F003-AC-02** — build Release x64 PASS.  
**F003-AC-03** — F001/F002 regression tests permanecem PASS.  
**F003-AC-04** — NetSDK externo detectado sem entrar no Git.  
**F003-AC-05** — core público permanece sem Qt/NetSDK.  
**F003-AC-06** — runtime NetSDK tem RAII e cleanup determinístico.  
**F003-AC-07** — login/logout não expõe senha em logs.  
**F003-AC-08** — dispositivo pode ser adicionado em memória.  
**F003-AC-09** — dispositivo pode ser conectado/desconectado.  
**F003-AC-10** — canais são derivados de login real/fake controlado, não hardcoded na UI.  
**F003-AC-11** — árvore representa dispositivo e canais reais.  
**F003-AC-12** — falha de login produz diagnóstico sanitizado.  
**F003-AC-13** — shutdown não deixa sessão NetSDK ativa.  
**F003-AC-14** — teste real em equipamento Intelbras por IP/LAN PASS.  
**F003-AC-15** — reconnect controlado PASS.  
**F003-AC-16** — zero processo residual após ciclos.  
**F003-AC-17** — nenhum componente Cloud/P2P/PlaySDK/vídeo foi introduzido.  
**F003-AC-18** — nenhuma alegação de produção/redistribuição Intelbras é feita.

---

## 8. ADRs

- ADR-004 — client-framework: permanece Proposed.
- ADR-010 — interoperability-intelbras: permanece Proposed durante a execução.

A F003 poderá produzir evidência para futura decisão sobre ADR-010, mas não o promove automaticamente.

---

## 9. Resultado permitido

Executor técnico pode declarar somente:

- `FOUNDATION-003 — PASS / READY FOR HUMAN REVIEW`
- `FOUNDATION-003 — FAIL`
- `FOUNDATION-003 — BLOCKED`

`HUMAN REVIEW APPROVED` permanece decisão exclusiva do responsável pelo projeto.
