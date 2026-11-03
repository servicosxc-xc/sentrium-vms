# SENTRIUM VMS — F005 AutoRegistro recovery / UI RC02

**Data:** 01/11/2026. **Motivação:** operador detectou que NVRs já autenticados no SENTRIUM às vezes não voltam ao estado conectado após queda/retorno, a menos que altere e salve a configuração de Registro Automático no equipamento. No SIM Next, o operador não precisa fazer isso.

## Causa confirmada no código anterior

1. Em `libs/sentrium-intelbras/intelbras.cpp`, callbacks `CLIENT_InitEx` (disconnected) e `CLIENT_SetAutoReconnect` (reconnected) existiam **com corpo vazio**. Assim, o SENTRIUM não sabia do status da sessão NetSDK.
2. `DeviceWorker::onRegistrationReceived()` retornava sem ação quando `sessions_` tinha um handle e não estava marcado stale. O NetSDK podia manter um handle obsoleto no aplicativo por tempo indeterminado.
3. `DeviceWorker::onRegistrationReceived(disconnected)` às vezes conservava um handle antigo em `stale_registration_sessions_` após falha de Logout, impedindo novas autenticações; não existia watchdog nem retry automático.
4. `DeviceWorker::startLive()` rejeitava slots >= 16, embora o `DeviceController` e QML já oferecessem até 64.

**Nuance NetSDK:** guias de programação do fabricante indicam que o login do tipo Auto Registration/Server Conn não possui as mesmas garantias de reconexão automática do login TCP. Portanto **não confiar somente** no callback `CLIENT_SetAutoReconnect`; realizar tentativas controladas no Worker usando o ID e endpoint observados no listener.

## Correção de código

- `Adapter::setSessionStatusCallback`: propaga desconexão e retorno do NetSDK com handle opaco; callbacks de SDK nunca chamam Login ou Qt GUI diretamente, apenas copiam a função sob mutex e enviam aviso ao `DeviceWorker`. Exceções são retidas na fronteira nativa.
- `DeviceWorker`: mapeia handle→dispositivo registrado, marca sessão offline no aviso SDK ou listener, interrompe vídeo com `stopLiveForDevice`, notifica Controller para estado Connecting, mantém o handle quando ainda é elegível à reconexão SDK e aceita um novo Registro Automático mesmo com handle velho.
- `DeviceWorker`: guarda somente na memória o último endpoint de Registro Automático de dispositivos explicitamente cadastrados; `QTimer` na thread worker tenta login de recuperação a cada 15s para registros pendentes. Reusa as credenciais já cadastradas, **sem instruir configuração do NVR, sem inventar IP/ID e sem salvar segredos em logs**. A recuperação manual usa `EM_LOGIN_SPEC_CAP_SERVER_CONN` e reconstrói a sessão mesmo se `CLIENT_Logout` da sessão perdida falhar. Não cria dois logins em paralelo para um dispositivo.
- `DeviceWorker`: eventos positivos duplicados não reinicializam uma sessão saudável; estados persistentes são limpos no stop/unregister/shutdown.
- `DeviceController`: preserva uma fotografia dos slots/streams (posição, índice, Extra/Principal) na primeira perda de conexão; quando `connectFinished` confirma nova sessão real e canais, reabre automaticamente os slots ainda livres. Uma desconexão intencional apaga esta intenção; callbacks atrasados não ressuscitam o dispositivo removido.
- `DeviceWorker::startLive` agora aceita 64 slots.
- Sem modificar protocolo Cloud/P2P, IP do listener, cadastro automático de desconhecidos, ou a especificação privada do ID de registro.

## Correções da interface consolidadas

Além das oito alterações r25 já presentes na RC01, esta RC02 contém as melhorias posteriores: abas aproximadas do nome SENTRIUM e sem atalhos fixos de Nova Visualização/Reprodução, pesquisa lateral em cor clara, menu por botão `⋯` com popup Window sobre vídeo nativo, e arraste tile→tile pelo cabeçalho por coordenada física. `CameraContextMenu.qml` preserva workaround do gerador de QML do MSVC/Qt 6.12 (`QT_QML_SKIP_CACHEGEN`).

## Gate real — NÃO inferir dos testes estáticos

1. Primeiro compile no Windows com Qt 6.12.0 + MSVC 17.14 + NetSDK/PlaySDK existentes. O código no GitHub passou inspeção de estrutura e conexões, **mas não build do Windows nem teste com gravador real**.
2. Ative listener na mesma IP/porta existente. Cadastre gravador apenas se necessário. Aguarde 1 login real e identificação de canais.
3. Abra 2 câmeras (uma Extra, outra Principal). Simule indisponibilidade **reversível** da rede/gravador, aguarde retorno **sem abrir nem salvar as configurações de Registro Automático do dispositivo**.
4. Critério esperado: UI muda Connected→Connecting→Connected; no retorno, canais e posições anteriores voltam automaticamente; não cria gravador duplicado; outros gravadores não caem. A recuperação pode levar pelo menos um ciclo de 15 segundos, mais os timeouts da NetSDK.
5. Repita interrupção/retorno duas vezes, teste reinício de listener com servidor ativo e teste desconexão voluntária (não deve reconectar sem novo comando).
6. Com 4, 9 e 16 tiles, validar menu `⋯`, clique direito no cabeçalho, mover canal de tile, menu contextual e botão direito sobre superfície native HWND (não assumir que mouse sobre vídeo funciona sem ponte nativa).
7. Se ao voltar o NVR não estabelecer reverse connection nem os callbacks chegarem, coletar log sanitizado de diagnóstico + timestamps/estados. Em particular, verificar se o último endpoint reportado pelo fabricante pode ser usado no LoginEx2 em reconexão, em vez de assumir que todo firmware suporta recuperação pelo mesmo socket.

**Segurança:** somente dispositivos de Registro Automático explicitamente cadastrados são aceitos. Nenhum ID completo/peer/senha é adicionado a logs. Código versionado na branch de desenvolvimento, sem merge em `main` ou F005.

**Resultado:** PENDENTE da validação Windows/campo. Esta é uma correção implementada, **não uma prova de reconexão já funcional**.
