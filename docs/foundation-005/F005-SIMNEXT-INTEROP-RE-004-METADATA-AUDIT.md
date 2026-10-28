# F005 RE-004 — Auditoria reproduzível do início de sessão Cloud P2P

Data de pesquisa: 28/10/2026. Fonte: duas capturas de uma instalação legítima do SIM Next conectada a gravador autorizado, previamente analisadas em RE-002 e RE-003. Conclusões abaixo derivam exclusivamente de metadados observáveis; **não temos especificação oficial do Cloud nem autenticação de terceiro aprovada**.

## Resultado técnico novo

A segunda captura começou com o SIM Next fechado e foi gravada durante inicialização, conexão P2P, abertura de um canal e fechamento da visualização. O usuário confirmou o rótulo **P2P** na interface. O IP WAN público do local do DVR coincidiu com o destino de grande volume em ambas as capturas RE-002/RE-003 (e é omitido nesta documentação por privacidade).

Analisamos novamente o arquivo PCAPNG RE-003, com um **novo analisador reprodutível que não depende de Wireshark, TShark, Scapy ou rede**. A leitura direta dos blocos Enhanced Packet Block calcula **5.135 quadros em 73,689 segundos**; a nota anterior estimava 73,320 s. Para futuros relatórios, usar a duração recalculada e reprodutível, sem alterar as conclusões sobre os eventos relativos a t≈41–70s.

### DNS e HTTPS observados, sem decifrar o TLS

| Momento relativo | Evidência de rede |
|---|---|
| 11,252 s | Consulta DNS do nome público `api-cronos.intelbras.com.br` |
| 11,260 s | TCP SYN porta 443 a um endereço retornado na resposta DNS desse nome |
| 11,260 s | Consulta DNS do nome público `intelbrasp2p.com.br` |
| 12,641 s | Consulta DNS `api.simnext.isec.intelbras.com.br` |
| 12,662 s | TCP SYN porta 443 a um endereço retornado na resposta desse nome |
| 14,118 s | Consulta DNS `cronos-simnext.intelbras.com.br` |
| 14,423 s | Consulta DNS `logs.isec.intelbras.com.br` |
| 14,429 s | TCP SYN porta 443 a um endereço retornado na resposta desse nome |

A correspondência DNS→TCP confirma contato com alguns serviços da Intelbras durante a inicialização. **Não afirma que esses servidores autenticaram P2P**. DNS e HTTPS também podem servir à configuração, telemetria e funções não relacionadas ao Cloud. O DNS `intelbrasp2p.com.br` foi observado, mas isso **não** equivale a confirmar um handshake de autenticação dessa plataforma ou uma API pública utilizável pelo SENTRIUM.

### Tentativas e estados UDP na rede autorizada do DVR

| t aproximado | Direção observada e marcador inicial do datagrama |
|---|---|
| 40,818 s | Local → rede DVR, porta remota UDP 1132, `12 20 D0 07` (184 bytes de UDP) |
| 41,420 s | Local → rede DVR, porta remota UDP 1131, `12 20 D0 07` (184 bytes) |
| 41,494 s | DVR → local, UDP 1131, `12 20 D0 07` (184 bytes); local → DVR, `12 20 D1 07` |
| 41,520 s | DVR → local, `12 20 D2 07` (24 bytes) |
| 42,055–42,078 s | Nova troca `D0`/`D1`/`D2` em UDP 1131; pode ser segunda negociação, sem interpretação definida |
| 42,084–42,110 s | Local `12 01 10 00` → resposta `12 01 10 01` |
| 42,125–42,155 s | Local `12 01 10 05` (1.400 bytes) → resposta `12 01 10 06` (16 bytes) |
| 42,175–42,203 s | Local envia `12 01 10 02`; rede remota responde `12 01 10 03` |
| 56–70 s | Tráfego volumoso compatível com vídeo do canal aberto; redução após o fechamento |

**Achado importante:** o primeiro contato usou porta UDP remota 1132; o fluxo bidirecional que persistiu usou a porta remota **1131**, com porta local 51699. Em capturas diferentes, as portas remotas mudaram (RE-002 usou 1639). Isso é compatível com seleção/negociação de candidatos de transporte, **não é comprovação suficiente de UDP hole punching, mapeamento NAT específico ou mecanismo criptográfico**.

Na RE-003, o candidato principal apresentou 3.964 quadros e 3.586.029 bytes Ethernet. O marcador de dados `12 01 10 02` foi observado em 3.254 datagramas, todos com o campo de comprimento (bytes 8–9) consistente com um cabeçalho externo de 16 bytes. O mesmo teste aplicado à RE-002 passou em **6.481/6.481** datagramas. É um formato observável, não um protocolo autorizado de login. O parser C++ experimental PTCP existente no SENTRIUM espera `PTCP`/24 bytes, portanto não equivale a esse envelope.

## Nova ferramenta de análise — reprodutível e sanitizada

Arquivos:

- `scripts/research/sentrium-p2p-pcap-audit.py`: leitura local PCAPNG/Ethernet/IPv4/IPv6/TCP/UDP, DNS público de nomes previamente aprovados, associação de endereços DNS em memória a TCP SYN 443, identificação de conversa UDP dominante, prefixos conhecidos, durações, períodos de bytes/segundo e precursores no mesmo host remoto; **sem gravar IPs, MACs ou payloads**.
- `tests/research/test_re004_pcap_audit.py`: 4 testes com PCAPNG **inteiramente sintéticos**, verificação de integridade, vazamento de IP, regras de DNS, framing e reconhecimento de portas.
- `scripts/windows/sentrium-audit-f005-pcap.ps1`: invocação de uma linha a partir do repositório, com Python 3 (biblioteca padrão), escrita do JSON sanitizado somente em `out/simnext-interop/` (ignorado no Git).

Comando Windows para eventual auditoria de **uma captura local que já exista**, sem nova coleta:

```powershell
.\scripts\windows\sentrium-audit-f005-pcap.ps1 -PcapPath "C:\CAMINHO\captura.pcapng"
```

**Validação executada no ambiente de desenvolvimento:** Python `unittest` **4/4 PASS**; RE-002 PCAPNG real **15.087 frames/48,957 s**, conversa dominante 4.508 frames/5.484.597 bytes, UDP remoto 1639/local 64220; RE-003 PCAPNG real **5.135 frames/73,689 s**, conversa dominante 3.964 frames/3.586.029 bytes, remoto 1131/local 51699 e precursor remoto 1132. Campos de comprimento dos type02 foram 6.481/6.481 em RE-002 e 3.254/3.254 em RE-003. Verificou-se que os JSONs sanitizados **não contêm o IP público do DVR nem o IP local do operador**.

**Não executado em Windows ainda:** o wrapper PowerShell recém-criado e a execução Python no Windows do operador. Isso não altera código C++ nem exige repetir R10. Os dados PCAPNG privados originais jamais são versionados.

## Decisão técnica e próximo gate

**Confirmado:** tráfego UDP de vídeo até o WAN real do local do DVR durante sessão que o SIM Next rotula P2P; troca de datagramas de negociação antes de transmitir vídeo; contatos com serviços DNS/HTTPS Intelbras no startup; framing UDP de 16 bytes com marcadores recorrentes.

**Não confirmado:** função/credenciais das APIs HTTPS, associação do pedido de Cloud a um serial, identidade do aplicativo, autenticação/consentimento do gravador, algoritmo de descoberta oficial, garantia de conexão direta sem auxiliares, suporte a um programa de terceiros. **Jamais enviar os datagramas observados como se isso concedesse direito de autenticar o SENTRIUM.** A próxima implementação de um provider de produção depende de documentação/autorização própria ou de uma arquitetura independente explicitamente aprovada; o pedido ao fornecedor permanece em `F005-INTELBRAS-CLOUD-APP-ACCESS-GATE.md`.

RE-004 não requer que o operador repita capturas. O foco posterior deve ser uma avaliação de interfaces públicas de sessão/autorização oficialmente disponíveis, distinguindo autenticação da infraestrutura e transporte já observável. Não copiar arquivos SIM Next, extrair tokens/chaves, desabilitar Defender ou executar `dh-fwd_win_x64.exe` bloqueado.

Governança: PR #4 draft, `main` e F004 Human Review intactos. R10 C++ 6/6 Windows PASS permanece a última validação do código nativo.

## R1 hotfix Windows — launcher Python com registro obsoleto (28/10/2026)

Teste real do operador falhou no wrapper: o comando py -3 apontava para um executavel que nao existe mais em uma instalacao Python 3.13 per-user. A captura PCAPNG foi localizada corretamente; a analise nao chegou a executar. Nenhum PASS foi declarado. **E um defeito de selecao do runtime Windows, nao uma falha de protocolo ou do gravador.**

O wrapper PowerShell foi corrigido para testar efetivamente cada candidato com `-c` e verificar versao >=3.9 + exit-code antes de invocar. Em caso de launcher `py` obsoleto, tenta python/python3 no PATH, versoes especificas do launcher e caminhos existentes em `%LOCALAPPDATA%\Programs\Python\Python*\python.exe`. Permite `-PythonPath` para informar o executavel real. Ignora aliases WindowsApps para evitar abrir a Microsoft Store. Caso nao exista Python funcional, retorna erro com diagnostico e **nunca inventa resultado PASS**. O relatorio privado e escrito somente sob out/, preservando a politica de privacidade.

Comando de reteste no PowerShell do operador:

```powershell
cd 'C:\Projetos\SENTRIUM VMS\sentrium-vms'; git pull --ff-only origin foundation/005-device-connectivity; if ($LASTEXITCODE -eq 0) { .\scripts\windows\sentrium-audit-f005-pcap.ps1 -PcapPath 'C:\Users\ericx\Downloads\wireshaark 2.pcapng' }
```

Validacao Python 4/4 sinteticos anterior nao valida a resolucao do Python no Windows do operador. O wrapper atualizado **ainda precisa de confirmacao de execucao Windows**. C++ R10 Windows 6/6 PASS continua intocado, PR draft e main/Human Review preservados.

## TShark fallback Windows (28/10/2026)

O operador validou diretamente com TShark no Windows: 3964 pacotes e 3586029 bytes UDP (porta 1131) na RE-003, concordantes com o analisador Python. Python 3.9+ nao existe no PC e o launcher py -3 esta obsoleto. O script windows principal agora tenta TShark automaticamente quando nenhum Python funcional for encontrado. Ele processa conversas UDP IPv4 por enderecos/portas SOMENTE em memoria e salva JSON sanitizado com contagens, bytes e portas (nenhum IP ou payload). Schema distinto sentrium.f005.pcap.tshark-transport.v1 e status PASS_PARTIAL_TRANSPORT_ONLY. E uma validacao PARCIAL de transporte: NAO executa DNS/HTTPS, envelope 16B, autenticao Cloud nem demonstracao de acesso independente. Portas sao especificas da sessao. Essa rotina nova ainda precisa de validacao no PowerShell do operador; o TShark em si ja foi comprovado. Sem alteracoes C++/CTest, R10 anterior continua 6/6 PASS, PR draft e main/F004 Human Review intactos.
