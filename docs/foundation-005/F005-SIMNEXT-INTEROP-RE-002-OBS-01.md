# SENTRIUM F005 — RE-002 OBS-01: Primeira captura Wireshark, candidato UDP de vídeo

Data local 27/10/2026 (captura enviada em conversa). Origem: imagem Wireshark, aba TCP Conversations selecionada (33 conversas TCP; indicador de 234 UDP), e texto exportado com campos número, tempo relativo, src/dst, protocolo, tamanho e indicação de portas. Exportação interpretável: 4.707 linhas, t=0..13,157 s, índice de último pacote 4747 (alguns pacotes não incluídos no texto). Observação autorizada; nenhum pacote bruto, IP de cliente/servidor, serial, pseudonym salt ou payload registrado aqui.

## Prova observacional

- Texto contém 4.424 datagramas UDP, 233 linhas interpretadas como TCP e outras 50 linhas de DNS/TLS/Gryphon/mDNS/MNDP. Soma de tamanhos dos datagramas UDP no texto: 2.369.454 bytes (inclui cabeçalhos de enlace/IP/UDP apresentados pelo Wireshark).
- Uma conversa UDP de portas 1639 (remota) e 64220 (local) domina a exportação: 1.737 datagramas nos dois sentidos e 2.007.020 bytes totais (~84,7% dos bytes UDP). No sentido remoto -> local: 1.546 pacotes e 1.994.186 bytes; local -> remoto: 191 pacotes e 12.834 bytes.
- Padrões: 1.197 pacotes de 1.442 bytes e 151 pacotes de 946 bytes no par UDP; indica datagramas grandes repetidos, sem estabelecer que sejam H.264/H.265 ou protocolo Cloud específico.
- Nos intervalos 0–1s / 1–2s / 2–3s / 3–4s, a conversa movimentou 1.080 / 124 / 536 / 1.072 bytes respectivamente. Em 4–5s saltou para 222.226 bytes. Os primeiros pacotes grandes (>900 bytes) do sentido remoto para local aparecem aproximadamente em t=4,656 s.
- Entre 5 e 12 s, a conversa sustentou ~206–239 kB/s incluindo overhead de captura, equivalente a aproximadamente 1,65–1,91 Mb/s. Os demais fluxos UDP no período totalizaram ~362.434 bytes, muito menos relevantes.
- Um evento outbound da porta local 64220 para a remota 1639 de 220 bytes aparece em t=4,121s e outro de 926 bytes em t=4,242s, antes do início dos datagramas grandes em t=4,656s. **Correlação temporal apenas**, não prova handshake ou autenticação.
- Um conjunto de aproximadamente 219 UDP datagramas em 50 ms próximo de t=6,23s aparece distribuído entre vários endereços remotos, possivelmente manutenção/controle sincronizado; não atribuir à autenticação ou vídeo sem confirmação.
- Na imagem foi selecionada a ABA TCP (33 conversas), portanto não aparece a conversa UDP dominante na tabela visível. A exportação registra 226 conversas UDP, enquanto a captura visual exibe 234; podem ter diferenças de filtro/estado da análise. Não assumir que os recortes constituem capturas completas idênticas.

## Interpretação controlada

**Hipótese forte:** stream de vídeo remoto usando transporte UDP na conversa entre as portas 1639 e 64220, com estabelecimento ou início de fluxo contínuo aproximadamente t=4,6 segundos. Evidência temporal e tráfego unidirecional de ~1,8 Mb/s, compatíveis com substream. NÃO comprova que a conexão era Direct P2P, Relay ou IP tradicional, nem identifica se o host remoto era NVR próprio, outro equipamento ou infraestrutura de terceiro. A identidade do endpoint, autenticação, criptografia e dados de camada de aplicação não foram examinados. Sem confirmação do operador se a câmera realmente apareceu na UI e qual rota o SIM Next mostrou.

**Limite da RE-002 OBS-01:** a exportação cobre somente 13,157s, não a janela planejada de 40s com baseline antes, vídeo ativo e encerramento após fechar câmera. Como o candidato já está ativo ao fim da captura, falta o momento de desligamento do stream para prova causal. Também não há associação PID/socket Windows suficientemente individualizada para a porta UDP 64220.

## Próxima coleta mínima (requer ação manual no PC)

1. Primeiro abrir a ABA UDP de Statistics > Conversations; ordenar Bytes decrescente. A conversa com a maior contagem de bytes deve destacar portas UDP 1639 ↔ 64220 se for a mesma captura.
2. No Wireshark, filtro de exibição: udp.port == 1639 && udp.port == 64220 ; isso restringe ao candidato pelo par de portas. Preferir inserir também IP remoto de forma local se o par de portas não for único, mas não publicar endereço em GitHub/chat.
3. Em uma captura NOVA de aproximadamente 40 segundos, deixar o SIM Next com nenhum tile aberto por ~10s; abrir uma câmera remota EXTRA por ~20s; fechar e manter o registro por ~10s; observar se o volume do candidato cai ou desaparece após fechar.
4. Registrar apenas o segundo aproximado de abertura/fechamento, vídeo visível OK/erro e exatamente como a UI apresentou Cloud (P2P)/Cloud (Relay)/IP-Domínio/não mostrado. Esta informação não está no pacote.
5. Enviar screenshot redigido de Statistics > Conversations > UDP e gráfico Statistics > I/O Graphs de pacotes/bytes do filtro, omitindo IPs/serial/credenciais. Não enviar pcapng bruto, não instalar SIM Next DLL no SENTRIUM, não extrair tokens/chaves do fornecedor.

## Estado do projeto

Este achado distingue uma conversa com forte assinatura de mídia em UDP; não fornece API nem prova autorização ou implementação própria Cloud. Fundação C++ R10 build Windows PASS, D-41..D-46 6/6 PASS permanece; nenhum código C++/CTest/runner alterado; PR #4 draft, main e F004 Human Review intactos. Não executar dh-fwd Defender-blocked nem contornar autenticação.