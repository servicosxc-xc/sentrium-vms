# SENTRIUM-CPP-Foundation-005-SCOPE 0.1 — DEVICE CONNECTIVITY

**Data:** 25/10/2026  
**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Branch:** `foundation/005-device-connectivity`  
**Autorização humana:** **F005-AUTH-001 — AUTHORIZED FOR EXECUTION**, confirmação do responsável “vamos seguir” à proposta de priorizar interface, Cloud/P2P e Registro Automático.  
**Estado:** **AUTHORIZED / IMPLEMENTATION PENDING LOCAL EXECUTOR**.

## Objetivo verificável

Abrir o `sentrium-client`, adicionar um gravador real por **LAN, Cloud/P2P ou Registro Automático**, estabelecer sessão apropriada, exibir estado verdadeiro e enumerar seus canais reais, permitindo desconectar/remover sem processos remanescentes. **Não** afirmar funcionamento de modalidade não implementada.

## Base e limites

- Reutilizar UI Qt 6.12, `DeviceController`, `DeviceWorker`, `sentrium-device` e `sentrium-intelbras` já existentes; LAN está operacional.
- Branch iniciada de `715b81912c0ba33508bca3fba284fee023bc0fee` (**F004 ainda não fechada**), sem merge em `main`. Esta dependência deve ser resolvida em revisão humana posterior; criar a branch não aprova F004.
- **F004 fica em pausa para benchmarks de escala**: somente duas câmeras úteis no NVR local. Testes reais de 4/16/32 câmeras são `NOT RUN` neste hardware. Não repetir testes de CPU/GPU/handles como pré-requisito à entrega de conectividade.
- P2P: investigar o caminho legado `SIMNext.P2PSDK.dll → Libt2uWrapper.dll → libt2u.dll` e os contratos efetivamente disponíveis. Não portar indiscriminadamente o legado nem inventar exports.
- Registro Automático: investigar APIs de serviço/listen/registration do NetSDK **conforme headers/documentação do kit presente**, com origem, callback, autenticação, teardown e controle de portas explicitados. Não inventar ABI.
- Dependências proprietárias e dados operacionais ficam fora do Git; não realizar downloads nem instalações automáticos.

## Entregas em sequência

1. **F005.1 — Interface operável**: escolha LAN / Cloud-P2P / Registro Automático com campos específicos, validação, estados e erros reais; preservar LAN. Não mostrar “Connected” sem uma sessão real. Opções sem SDK comprovado devem indicar claramente indisponibilidade e não enviar requisições fictícias.
2. **F005.2 — Cloud/P2P real**: prova do transporte com dispositivo real e login NetSDK válido, enumeração de canais, desconexão e reconexão. Distinguir P2P direto (azul) e Relay (amarelo) somente quando SDK reportar a modalidade; desconhecido fica neutro.
3. **F005.3 — Registro Automático real**: serviço explicitamente configurado e desativado por padrão, associação a DeviceId lógico, entrada/saída do dispositivo, enumeração e desconexão/stop. Listener de rede só com configuração/autorização local explícita; não abrir firewall, NAT ou serviços públicos automaticamente.
4. **F005.4 — Validação funcional concentrada**: uma prova funcional LAN e, quando disponíveis, uma prova Cloud e Registro Automático em equipamento real, incluindo gravadores 16/32 canais acessíveis, com log sanitizado. Mídia/live-view definitivo virá depois.

## Requisitos não negociáveis

- Não persistir usuário/senha/serial sensível sem decisão explícita; minimizar vida útil de segredos na memória e sanitizar logs.
- Não acoplar ciclo de vida do túnel P2P ao RealPlay; sessões liberadas e conexões fechadas deterministicamente.
- Mesma identidade lógica de dispositivo e árvore de canais independentemente do transporte.
- Não introduzir broker/framework genérico sem consumidor, nem registros de câmera inventados, fallback de transporte silencioso ou estados falsos.
- Nenhuma alteração da baseline vcpkg, Qt, tags aprovadas ou `main`; sem alterar a F004 retroativamente.
- Nenhum merge ou `HUMAN REVIEW APPROVED` sem revisão humana explícita.

## Fora do escopo

Live View de produção, render Qt Quick/mosaico, playback, gravação, IA/LPR, banco completo, installer, benchmarking multicâmera de escala sem equipamento real e infraestrutura de serviços de exposição pública.

## Critérios de aceite

- **F005-AC-01** — build Debug/Release e regressão mínima do que foi afetado.
- **F005-AC-02** — UI de três métodos e fluxo LAN preservado.
- **F005-AC-03** — Cloud/P2P usa API e binários reais verificados, ou modalidade é explicitamente `BLOCKED` (não `PASS`).
- **F005-AC-04** — Cloud real enumera canais e retorna estado de conexão correto.
- **F005-AC-05** — P2P direto/Relay somente quando observado; estado desconhecido neutro.
- **F005-AC-06** — Registro Automático usa API real verificada e um dispositivo real; ou `BLOCKED`, sem simulação.
- **F005-AC-07** — credenciais não aparecem em evidências e não são persistidas.
- **F005-AC-08** — disconnect/reconnect/remove limpos e sem vazamento/processo residual no caso validado.
- **F005-AC-09** — árvore representa número e canais reais do equipamento.
- **F005-AC-10** — relatório distingue `PASS`, `NOT RUN`, `BLOCKED` e dependência externa para cada modalidade.

**Resultado máximo do executor:** `FOUNDATION-005 — PASS / READY FOR HUMAN REVIEW` somente se os critérios relevantes estiverem comprovados. Aprovação humana posterior.
