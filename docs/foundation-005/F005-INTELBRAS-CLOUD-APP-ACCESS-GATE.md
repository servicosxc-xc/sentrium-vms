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