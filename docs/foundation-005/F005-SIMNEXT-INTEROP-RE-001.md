# F005 RE-001 — Observacao de interoperabilidade do SIM Next

Data: 27/10/2026. Branch foundation/005-device-connectivity, PR #4 draft. Estado: instrumento entregue, **coleta real ainda NOT RUN**.

## Objetivo e limites

Investigar viabilidade de um cliente SENTRIUM Cloud por serial, independente do SIM Next/Defense IA. Usar SIM Next legitimamente instalado, gravador sob posse/consentimento, e rede controlada. Primeiro coletar metadados passivos, depois analisar **apenas** trafego relevante com ferramentas de rede convencionais. Nao depurar/alterar codigo de terceiros, extrair chaves, simular identidade de aplicativo, contornar autenticacao, desativar protecoes de seguranca ou distribuir binarios. Se houver exigencia de credenciais/licenca especificas do provedor, continuar pela via oficial antes de integrar o servico comercialmente.

A referencia de protocolo publico e https://manuais.intelbras.com.br/manual-sim-next/simnext.html : o manual diferencia IP/Dominio, Cloud (P2P) e Cloud (Relay), com tentativas de ligacao direta TCP/UDP e fallback Relay. **O modo verdadeiro vem da interface do SIM Next**, nao de uma inferencia baseada na porta ou latencia.

A API Defense IA 3.3 tambem reconhece dispositivo addType=5 (P2P), mas a API REST do Defense e diferente do bootstrap Cloud por serial de aplicacoes independentes; nenhum protocolo proprietario ou direito de acesso e deduzido.

## Ferramenta criada no repositorio

Arquivo: scripts/windows/sentrium-observe-simnext.ps1. Somente leitura de:
- Nomes/PIDs dos processos SIM Next e filhos ja existentes; assinatura, versao e SHA-256 dos executaveis, se legiveis.
- Nomes de DLLs carregadas, **nao** suas copias/conteudo.
- Conexoes TCP (PIDs, portas, estado, identificadores pseudonimos dos enderecos remotos), endpoints **locais** UDP e instantes relativos.
- Sal aleatorio persistente em out/simnext-interop/local-hash-salt.txt para comparar destinos entre varias observacoes sem registrar seu IP literal. NAO enviar salt ou PCAP bruto.
- Relatorios JSON locais em out/simnext-interop, ignorados pelo Git (.gitignore /out/). Pseudonimizacao nao e anonimato total; revisar metadados antes de enviar.

O script **nao** captura payload, TLS, DNS, senha, numero de serie, tokens, certificados, memoria ou historico do navegador. Nao controla SIM Next e nao abre sessoes: o operador clica normalmente. Get-NetUDPEndpoint NAO revela o endereco do peer UDP; para isso seria necessaria captura Wireshark separada. A arvore de PIDs e coletada no inicio: helpers criados depois podem precisar de uma segunda execucao com -ProcessIds.

## Experimento inicial

1. Em um PC Windows com SIM Next instalado legitimamente, abra o software e deixe **somente um gravador autorizado de teste** pronto para Cloud. Nao compartilhe o serial nem credenciais.
2. No Developer PowerShell, dentro de C:\Projetos\SENTRIUM VMS\sentrium-vms, execute:
   .\scripts\windows\sentrium-observe-simnext.ps1 -Scenario Connecting
3. O script identifica os PIDs. Se nao encontrar, executa instrucao para consultar nomes de processos e permite -ProcessIds 1234 com o PID real.
4. Quando surgir a pergunta, pressione ENTER e inicie a conexao Cloud na interface do SIM Next. A amostragem dura 25 segundos (configuravel com -DurationSeconds e -IntervalMs).
5. Registre manualmente o resultado visivel do SIM Next: Cloud (P2P), Cloud (Relay), IP/Dominio ou falha. Nao classifique como Relay sem confirmacao da UI.
6. Para comparar: executar de novo com -Scenario Idle, sem conexao; -Scenario Live ao abrir apenas uma camera no stream extra; opcionalmente -Scenario Relay somente quando outra sessao autorizada mostrar Relay de fato.
7. Localize o arquivo JSON em out/simnext-interop e revise antes de compartilhar. Nao envie local-hash-salt.txt e nao adicione arquivos de observacao ao GitHub.

A coleta nao necessita build C++ nem SDK, e nao deve rodar os antigos CTest F005. R10 D-41..D-46 ja PASS (1,38 s) no commit c211b964; aqui estamos estudando uma conexao **REAL**, diferente do teste de loopback.

## Segundo experimento condicionado ao primeiro — pacotes Wireshark

Se os metadados mostrarem quais PIDs/portas participam:
- Capturar na propria interface de rede sob controle com Wireshark, apenas durante a sessao de teste. Usar filtro de exibicao inicial udp || tcp || dns, depois restringir por horario/endpoint. Pacotes brutos podem conter seriais, tokens, destinos privados e comunicacoes sem relacao: nao enviar nem versionar PCAPNG sem revisar/remover material sensivel.
- Fazer no minimo transicoes sem sessao, conectar, live, desconectar; anotar tempos e status de rota visivel.
- Comparar com uma sessao Cloud (Relay) real se estiver disponivel legitimamente. Nao forcar Relay burlando firewall, contas ou servidores, e nao supor IP/porta fixos.
- Nao decifrar TLS ou extrair chaves. Os dados observaveis incluem endereco/porta remoto, protocolo, tamanho, timing, DNS e comportamento de reconnect, **nao necessariamente** o conteudo de autenticacao.
- Reproduzir analise com tabelas de fluxos e diagrama de estados. Identificar o que esta confirmado versus hipotese.

## Perguntas para o parecer de viabilidade

(1) Qual processo e quais DLLs (nome/assinatura) fazem a sessao? (2) Quais destinos TCP/UDP surgem quando conecta e na live? (3) Como o trafego se altera entre IP, P2P e Relay medidos pela UI? (4) Ha sinais de rendezvous, canal de dados, keepalive ou reconexao? (5) A identidade do cliente requer autorizacao proprietaria ou existem interfaces publicas suficientes? (6) Poderiamos usar somente identidades legitimamente obtidas para o SENTRIUM, sem executar dependencias SIM Next em producao?

**STOP:** se o unico caminho identificado exigir copiar chave/identidade do cliente oficial, desabilitar validacoes ou modificar componentes bloqueados pelo Defender, interromper essa via e retornar ao pedido de SDK/autorizacao formal (docs/foundation-005/F005-INTELBRAS-CLOUD-APP-ACCESS-GATE.md). Alternativa: gateway gerenciado independente, se explicitamente escolhido pelo operador.

Governanca: main intocada, PR #4 draft, Human Review F004 pendente, sem nenhuma alegacao nova de Cloud funcionando.
