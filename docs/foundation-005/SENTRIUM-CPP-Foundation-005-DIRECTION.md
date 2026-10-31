# SENTRIUM-CPP-Foundation-005 — DIRECTION NOTE

**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Estado original (histórico):** DIRECTION ONLY — NOT AUTHORIZED FOR IMPLEMENTATION. **Posteriormente supersedido por F005-AUTH-001 — AUTHORIZED / IN IMPLEMENTATION.**  
**Data:** 25/10/2026

## Objetivo proposto

Transformar o cliente SENTRIUM em um gerenciador real de dispositivos antes de avançar para o Live View/mosaico definitivo.

A Foundation deverá permitir adicionar e operar dispositivos Intelbras/Dahua por três estratégias de acesso, preservando a mesma identidade lógica de dispositivo:

1. **LAN / IP direto**
2. **Cloud / P2P**
3. **Registro Automático / conexão iniciada pelo dispositivo**

## Interface proposta

A UI existente da F002/F003 evolui para um fluxo operacional mínimo:

- botão **Adicionar dispositivo**;
- escolha do tipo de conexão;
- cadastro de nome e parâmetros específicos;
- conexão/desconexão;
- árvore real de canais;
- status do transporte;
- remoção do dispositivo;
- mensagens de erro operacionais e sanitizadas.

### LAN
Campos atuais:
- IP/host;
- porta;
- usuário;
- senha.

### Cloud / P2P
Direção técnica:
- usar o caminho histórico `SIMNext.P2PSDK.dll → Libt2uWrapper.dll → libt2u.dll` **apenas como evidência de protocolo/comportamento**. Cloud no SENTRIUM será nativo e autônomo, sem exigir instalação, binários ou configurações do SIM Next;
- manter vida útil do túnel separada da vida útil do RealPlay;
- expor o resultado de transporte sem confundir com estado da câmera;
- preservar semântica visual futura: **azul = P2P direto**, **amarelo = Relay**.

### Registro Automático
Direção técnica:
- tratar como estratégia de transporte distinta de LAN/P2P;
- modelar dispositivo que inicia/estabelece o vínculo com o SENTRIUM/serviço de registro;
- não misturar o protocolo de registro com login/RealPlay;
- validar no SDK oficial quais APIs e callbacks são realmente necessários antes de implementar.

## Arquitetura proposta

Evitar framework genérico de plugins.

Usar uma abstração pequena de sessão/transporte suficiente para que o restante do produto enxergue:

- DeviceId lógico;
- estado de conexão;
- transporte atual;
- sessão NetSDK utilizável;
- quantidade/canais reais;
- disconnect/reconnect;
- diagnóstico sanitizado.

O pipeline de vídeo continua independente da estratégia que entregou a sessão.

## Fora desta direção inicial

Não incluir automaticamente:

- gravação;
- playback;
- timeline;
- IA/LPR;
- regras/eventos;
- banco completo;
- persistência de credenciais sem decisão explícita;
- mosaico definitivo;
- benchmark de 16/32 canais antes de haver gravadores realmente acessíveis.

## Razão da mudança de sequência

O NVR LAN atualmente disponível não possui quantidade suficiente de câmeras ativas para produzir benchmark representativo de escala.

Ao implementar Cloud/P2P e Registro Automático primeiro, o laboratório poderá acessar gravadores completos de 16/32 canais e então medir:

- ingest real;
- decode;
- render;
- CPU/GPU/memória;
- P2P direto versus Relay;
- escala Principal/Extra;
- comportamento de reconexão.

## Próximo passo documental

Antes de qualquer código desta Foundation:

1. auditar o caminho cloud legado e os binários/headers disponíveis;
2. auditar as APIs de registro automático do NetSDK;
3. escrever `SENTRIUM-CPP-Foundation-005-SCOPE`;
4. escrever `F005-DECISION-GATE`;
5. obter autorização explícita para implementação.

## Atualização vinculante da direção (após autorização F005)

Implementar Cloud/P2P standalone; jamais tratar os artefatos do SIM Next como dependência ou como fonte de credenciais de inicialização. Assinaturas/exportações citadas na auditoria ainda não constituem protocolo/ABI completa. Manter Cloud **BLOCKED** até bootstrap e caminho de controle próprios, de origem legítima e comprovada. A F005 agora está autorizada; critérios de qualidade e Human Review continuam vigentes.
