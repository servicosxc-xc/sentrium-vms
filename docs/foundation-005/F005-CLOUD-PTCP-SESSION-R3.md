# F005 Cloud R3 — PTCP state / ACK / bounded retry

Data: 26/10/2026. Branch `foundation/005-device-connectivity`, PR #4 draft.

Código C++20 independente em `cloud_ptcp_session.hpp/.cpp`: `Sent/Recv` contam bytes serializados do corpo PTCP, `PID` e `LMID` locais são gerados internamente, ACK cumulativo retira pacotes da fila; retransmissões preservam bytes, PID e LMID originais. Anti-replay para LMID repetido, rejeição de ACK regressivo/adiantado e detecção explícita de lacunas. A janela é limitada a 32 pacotes ou 64 KiB, intervalo de 400 ms e até quatro envios, com expiração segura. Sessão deve ser usada em uma única thread e receber relógio monotônico externo.

Não há reorder/reassembly, NAT, handshake autorizado, Direct/Relay, bootstrap de aplicativo, nem túnel TCP/NetSDK. Receber um datagrama PTCP **não significa** Cloud autenticado. As fontes públicas descrevem semântica de contadores (https://github.com/blackjid/dahua-p2p), mas não outorgam autorização de infraestrutura; nenhum código/binário externo foi incluído. Defender SECURITY HOLD mantido.

D-13/D-14 cobrem offline contadores, ACK, retransmissão, duplicata, gap, janela e timeouts. O script único do Windows realiza Debug build e D-11/D-12/D-13/D-14; D-9/D-10 não são repetidos. **BUILD e todos D-11 a D-14 NÃO EXECUTADOS NO WINDOWS** após este lote. Continuar ao receber evidência real; não proclamar Cloud funcional. Usuário e senhas de NVR/Cloud não entram em logs/Git.
