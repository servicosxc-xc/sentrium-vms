# SENTRIUM-CPP-Foundation-002 — RESULTADO

**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-002 — Application Bootstrap  
**Branch:** `foundation/002-application-bootstrap`  
**Estado técnico:** **PASS / READY FOR HUMAN REVIEW**  
**Data:** 25/10/2026

---

## 1. Resultado executivo

A Foundation-002 atingiu seu objetivo técnico: introduzir o primeiro cliente gráfico C++ real do SENTRIUM VMS com Qt Quick, preservando a separação do core, o toolchain aprovado e os limites de escopo.

O resultado técnico desta execução é:

**FOUNDATION-002 — PASS / READY FOR HUMAN REVIEW**

Este estado **não representa aprovação humana** e não promove automaticamente o ADR-004.

---

## 2. Entrega implementada

Foi criado o alvo `sentrium-client` com:

- Qt 6.12.0 — MSVC 2022 x64.
- Qt Core, Gui, QML, Quick e QuickControls2.
- Janela principal real.
- Identidade SENTRIUM VMS.
- Área de dispositivos vazia, sem dispositivos fictícios.
- Área de mosaico vazia, sem vídeo fictício.
- Abas `Visualizar` e `Reproduzir`.
- `Reproduzir` selecionável, porém explicitamente indisponível nesta Foundation.
- Painel `EVENTOS AO VIVO` vazio e recolhível.
- Barra de status.
- Versão, configuração e BuildId reais.
- Lifecycle e logging reutilizando a base da Foundation-001.
- DataRoot isolado em `%LOCALAPPDATA%\Sentrium\VMS\dev\Foundation-002`.

Nenhum SDK de câmera, FFmpeg, stream, decode, gravação, banco, IA, LPR, HTTP, WebSocket ou IPC foi introduzido.

---

## 3. PRE-FLIGHT e ambiente

O PRE-FLIGHT da Foundation-002 foi executado com sucesso no workspace permanente:

`C:\Projetos\SENTRIUM VMS\sentrium-vms`

Baseline validada:

- Visual Studio 2022 Build Tools 17.14.41.
- MSVC 19.44.35229.0.
- VCToolsVersion 14.44.35207.
- Host x64 / Target x64.
- Windows SDK 10.0.26100.9457.
- CMake 4.4.4.
- Ninja 1.13.2.
- vcpkg no baseline do manifesto.
- Qt 6.12.0 MSVC 2022 x64.
- Core / Gui / QML / Quick / QuickControls2 presentes.
- Plugin Windows do Qt presente em Debug e Release.

O aviso de ausência de `WrapVulkanHeaders` foi observado durante o configure. Vulkan não é dependência requerida pelo escopo desta Foundation e o configure concluiu normalmente.

---

## 4. Build

### Debug

- Configure: PASS.
- Build: PASS.
- `sentrium-client.exe`: gerado com sucesso.
- Interface gráfica: aberta e revisada manualmente.

### Release

- Configure: PASS.
- Build: PASS.
- `sentrium-client.exe`: gerado com sucesso.

Não são feitas alegações de desempenho de mídia, decode, renderização de múltiplos streams, IA ou LPR.

---

## 5. Testes automatizados

A suíte possui 20 testes por configuração:

- 8 testes unitários.
- 11 testes de integração da base.
- 1 `F002-CLIENT-SMOKE`.

O smoke gráfico executa internamente 10 ciclos de:

- abrir o cliente;
- localizar a janela principal;
- verificar DPI;
- redimensionar;
- fechar por mensagem normal de janela;
- confirmar ExitCode 0;
- validar eventos de startup/shutdown;
- confirmar ausência de processo residual.

### Debug

**20/20 PASS**

O `F002-CLIENT-SMOKE` também passou.

### Release

**20/20 PASS**

O `F002-CLIENT-SMOKE` também passou.

---

## 6. Revisão manual da GUI

Foi observado em execução real:

- janela principal renderizada;
- estado `Ready`;
- versão `0.0.0-foundation002`;
- BuildId exibido;
- aba `Visualizar`;
- aba `Reproduzir`;
- área de dispositivos vazia;
- mosaico vazio;
- painel `EVENTOS AO VIVO`;
- redimensionamento da janela;
- fechamento normal.

O log confirmou, em ciclo normal:

- `bootstrap-started`;
- `ui-initializing`;
- `window-presented`;
- `shutdown-started`;
- `shutdown-completed`.

---

## 7. Observação de idle — Release / 60 s

A observação é **descritiva**. Nenhum threshold de performance foi aplicado.

Resultados:

| Métrica | Observado |
|---|---:|
| Janela disponível após início | 239,260 ms |
| Shutdown | 49,648 ms |
| ExitCode | 0 |
| Duração idle | 60 s |
| Processadores lógicos | 16 |
| CPU acumulada durante idle | 437,5 ms |
| CPU média normalizada | 0,0456% |
| Working Set capturado no início | 54.845.440 bytes |
| Working Set ao fim | 87.666.688 bytes |
| Pico de Working Set | 88.985.600 bytes |
| Private Bytes capturado no início | 31.363.072 bytes |
| Private Bytes ao fim | 97.153.024 bytes |
| Processos residuais | 0 |

### Interpretação da memória

Os valores `workingSetStartBytes` e `privateStartBytes` foram capturados imediatamente após a janela tornar-se disponível, antes do assentamento completo do runtime gráfico.

Já no primeiro segundo, o processo estava aproximadamente em:

- Working Set: 88,64 MB.
- Private Bytes: 98,59 MB.

Do segundo 1 ao segundo 60, os valores permaneceram essencialmente estáveis e terminaram ligeiramente menores:

- Working Set final: ~87,67 MB.
- Private Bytes final: ~97,15 MB.
- Handles: 695 no início da amostragem e 690 no segundo 60.
- Threads: 66 no início da amostragem e 62 no segundo 60.

Portanto, o grande delta entre a captura instantânea inicial e a captura final **não deve ser interpretado como crescimento contínuo ou vazamento de memória durante os 60 s**. A amostragem mostra inicialização tardia/lazy do runtime Qt seguida por estabilidade no período observado.

Esta observação não substitui profiling prolongado ou análise de leak em etapas futuras.

---

## 8. Correções encontradas durante a execução

A execução real revelou e permitiu corrigir:

1. ambiente Developer PowerShell inicialmente aberto como x86;
2. preflight ainda preso ao workspace histórico da Foundation-001;
3. variável de vcpkg criada em processo PowerShell filho e não disponível ao CMake pai;
4. caminho inicialmente presumido incorreto para o executável;
5. runtime Qt não presente no PATH da sessão;
6. falso negativo do smoke test ao comparar tamanho externo da janela contra o mínimo da área cliente.

Essas correções foram feitas sem remover as travas de toolchain, arquitetura ou baseline.

---

## 9. Limites preservados

Continuam fora do escopo desta Foundation:

- Intelbras/Dahua NetSDK.
- PlaySDK.
- libt2u.
- Cloud/P2P.
- FFmpeg.
- captura de streams.
- decode de vídeo.
- renderização real de câmera.
- gravação.
- playback funcional.
- SQLite/banco operacional.
- autenticação/RBAC.
- credenciais.
- IA.
- LPR.
- eventos operacionais.
- regras.
- HTTP.
- WebSocket.
- IPC.
- installer.

Também não foram introduzidos:

- MessageEnvelope genérico.
- schema framework.
- event bus.
- command bus.
- plugin framework.

---

## 10. ADR-004

O uso de Qt 6.12.0 nesta Foundation continua sendo **experimental e reversível**.

**ADR-004 — client-framework: permanece Proposed.**

O sucesso do Application Bootstrap demonstra adequação do Qt Quick para este shell inicial. Não demonstra, por si só:

- capacidade de renderizar dezenas de streams;
- performance de decode/render de vídeo;
- eficiência de GPU em mosaicos;
- adequação definitiva do Qt à pipeline de mídia.

Essas decisões deverão ser suportadas por medições nas Foundations de mídia.

---

## 11. Estado técnico final

- PRE-FLIGHT: PASS.
- Debug build: PASS.
- Release build: PASS.
- Debug tests: 20/20 PASS.
- Release tests: 20/20 PASS.
- Smoke GUI: PASS em Debug e Release.
- Ciclos GUI: 10 por smoke/configuração.
- Startup/shutdown: observados.
- Idle 60 s: observado.
- Processo residual: 0.
- Escopo proibido: não introduzido.
- ADR-004: Proposed.
- Merge em `main`: não realizado.
- Aprovação humana: pendente.

# FOUNDATION-002 — PASS / READY FOR HUMAN REVIEW

A decisão de promover para **HUMAN REVIEW APPROVED**, aceitar/rejeitar ADR-004 ou autorizar qualquer Foundation posterior pertence ao responsável pelo projeto.
