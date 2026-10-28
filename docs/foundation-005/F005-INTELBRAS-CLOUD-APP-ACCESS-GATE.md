# F005 — Integração Cloud Intelbras por serial: gate externo de autorização

Data: 27/10/2026. Foundation-005 PR #4 draft. Documento de decisão e solicitação ao fornecedor; não há SDK Cloud fornecido ainda.

## Estado comprovado do código
R10 commit c211b964eefea6caa9f74c88bba9095ef9984e6d: PRE-FLIGHT PASS, CMake PASS, build Debug sentrium-cloud-network-tests [8/8] PASS; D-41 a D-46 6/6 PASS em 1,38 s. Rede TCP+PTCP+UDP real somente em localhost com peer de laboratório NÃO autenticado. Nenhuma sessão Cloud Intelbras real ou NVR remoto 16/32 canais autenticado foi comprovado.

## Referências públicas pesquisadas
- Procedimento Intelbras de solicitar SDK/API para DVR/NVR: https://backend.intelbras.com/sites/default/files/2022-08/api-sdk-seguranca-intelbras-whatsapp.pdf
- Tutorial do chatbot de suporte: https://backend.intelbras.com/sites/default/files/2023-05/procedimento-de-solicitacao-envio-sdkapi-atraves-do-bot-intelbras.pdf
- Canal de suporte: https://www.intelbras.com/pt-br/contato/suporte-tecnico/
- Resposta oficial em fórum de 03/07/2018 informa SDK Cloud por serial separado do SDK IP/DDNS e indisponível NAQUELA DATA: https://forum.intelbras.com.br/viewtopic.php?p=231915 . Não extrapolar política de 2018 para 2026.
- Manual atual SIM Next: https://manuais.intelbras.com.br/manual-sim-next/simnext.html distingue Cloud direto P2P de Relay.
- Portal Intelbras Moovsec Developers: https://developers.moovsec.com/ . Não há comprovação aqui de acesso por serial ao mesmo Cloud DVR/NVR. INC Cloud rede e GDI/Mibo não são substitutos confirmados.

## Solicitação pronta ao suporte Intelbras
Assunto: Consulta técnica/comercial — SDK Intelbras Cloud P2P para integração DVR/NVR por número de série (VMS C++ Windows x64)

Prezados, estamos desenvolvendo o SENTRIUM VMS, um sistema nativo C++20/Windows x64 que utiliza o NetSDK Intelbras para conexão IP e recursos de DVRs/NVRs. Buscamos oficialmente uma solução licenciada para conectar gravadores remotos pelo número de série Intelbras Cloud, com suporte a redes CGNAT sem encaminhamento de portas.

Solicitamos esclarecimentos:
1. Existe em 2026 SDK/API Cloud P2P autorizada para VMS de terceiros conectarem DVR/NVR por serial, distinta do NetSDK IP/DDNS?
2. Como solicitar acesso, credenciamento e licenciamento comercial? Há custos por conexão ou Relay?
3. Quais versões suportam integração C++ Windows x64 e coexistem com NetSDK? Existe código de exemplo de bootstrap e login aprovado?
4. Quais métodos oficiais de autenticação de aplicação, consentimento do dispositivo, estabelecimento P2P/Relay e verificação de identidade do peer?
5. O SDK permite criar um túnel de bytes/porta TCP local para login NetSDK ou exige API específica para live, replay, eventos e PTZ?
6. São suportados NVRs 16 e 32 canais e quais modelos/firmwares/limites de sessões?
7. Como identificar efetivamente a rota Direct P2P versus Relay, gerenciar timeouts e reconexão?
8. Moovsec Developers oferece esse mesmo acesso por serial a DVR/NVR, ou é uma plataforma de produto diferente?
9. Se não houver Cloud P2P disponível para terceiros, qual é a alternativa oficialmente suportada para conectar remotamente gravadores atrás de CGNAT?

Solicitamos encaminhamento à equipe de desenvolvimento de SDK/Cloud CFTV ou integração B2B e documentação vigente. Atenciosamente, Equipe SENTRIUM VMS.

## Critérios de liberação de implementação
Exigir documentação atual/licença de uso, identidade de app e endpoints emitidos legitimamente, autenticação mútua/session peer pinning, protocolo de Direct/Relay, contrato do túnel com NetSDK e acesso autorizado a NVR real para ensaio. Nunca registrar segredos/credenciais/número de série privado no repositório. Não usar DLLs SIM Next ou binário dh-fwd retido pelo Defender.

Se a autorização não existir, alternativas separadas: NetSDK por IP/DDNS/VPN, AutoRegistration com implantação permitida, ou gateway remoto gerenciado com autenticação mútua, criptografia e instalação consentida. Nenhuma alternativa é equivalente a Cloud por serial sem infraestrutura. Requer decisão de produto antes de desenvolver.

Próximo gate não é novo teste simulado. É resposta oficial de fornecedor ou decisão expressa sobre gateway seguro. PR draft, main e Human Review intactos.
## Revisao das fontes publicas e rota oficial de autorizacao (28/10/2026)

A pesquisa publica mais recente nao apresentou confirmacao de SDK Cloud Intelbras P2P por serial licenciado a aplicativos C++ de terceiros em 2026. As fontes abaixo sao factos publicados, nao evidencias de que o SENTRIUM esteja autorizado:

- Resposta oficial do suporte Intelbras no forum de **03/07/2018**: acesso pelo numero de serie Cloud necessitaria SDK especifico Cloud, diferente do SDK IP/DDNS, que **naquela data** nao era disponibilizado. https://forum.intelbras.com.br/viewtopic.php?p=231915 . **Nao presumir que politica de 2018 continue em vigor em 2026; consultar fornecedor.**
- Orientacao oficial de **16/10/2024**: para obter SDK/API convencional de gravador, contatar chat/WhatsApp no fluxo Ajuda com produtos > Seguranca > Cameras, gravadores e acessorios > Obter API/SDK. https://forum.intelbras.com.br/viewtopic.php?p=272216 . Esta orientacao nao afirma acesso Cloud por serial.
- Manual Intelbras Guardian Parceiro v1.6.0 consultado em 2026 explica metodos Cloud P2P, DDNS e IP, e que em alguns NAT/CGNAT o Relay pode ser usado. Confirma que existem aplicacoes oficiais de conexao por serial, mas nao implica SDK publico de terceiros. https://assets.guardian-partner.isec.intelbras.com.br/manual/html/pt-br/manual.html .
- Resposta oficial anterior no forum indica possibilidade de envio de termo de confidencialidade ao obter SDK ordinario, mas nao resolve a disponibilidade da integracao do serviço Cloud P2P: https://forum.intelbras.com.br/viewtopic.php?t=16383 .

**Decisao atual:** a solicitacao estruturada ja presente neste documento e o proximo gate externo adequado. Exigir resposta atual especifica para **SDK P2P por numero de serie, credenciais de aplicativo terceirizado, suporte a C++ Windows, conformidade/licenca comercial e caminho Direct/Relay**; o SDK NetSDK IP/DDNS nao deve ser apresentado como prova de Cloud. Nao enviar credenciais nem PCAPNG ao contato oficial. Nenhum pedido foi enviado pela automacao/assistente; depende de envio aprovado pelo operador.


## Atualizacao de decisao pelo operador — 28/10/2026

O operador informa que nao obtera SDK Cloud por chatbot Intelbras; o fluxo acessivel entrega somente SDK DVR/NVR convencional que ja foi disponibilizado ao projeto. **Nao insistir em solicitar novamente ao chatbot nem tratar resposta oficial como proximo passo obrigatorio da F005.** A possibilidade de uso licenciado de Cloud por serial continua tecnicamente NAO VERIFICADA para terceiros e **BLOCKED** (nao significa impossibilidade permanente). A continuidade executavel e AutoRegistration NetSDK (ja um NVR 8ch conectado), com transporte remoto seguro e infraestrutura controlada, e rede VPN/NetSDK como alternativa. A trilha de engenharia reversa Cloud continua passiva e independente, sem uso de segredos de SIM Next. Ver decisao `docs/foundation-005/F005-NO-VENDOR-CLOUD-SDK-PLAN.md`.
