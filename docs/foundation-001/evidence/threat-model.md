# SENTRIUM — threat model Foundation-001

O objeto protegido é a árvore exclusiva da Foundation resolvida por SHGetKnownFolderPath(FOLDERID_LocalAppData). Inputs do produto não escolhem caminhos de arquivo: a CLI tem comandos fechados e um identificador de fixture validado. `validate_root_input` rejeita qualquer override diferente do único root canônico. O produto não lê configuração histórica nem descobre diretórios legados.

Ataques cobertos: caminhos relativos, UNC, device paths, traversal, componentes ambíguos, nomes DOS reservados, alternate data streams, separadores em nomes filhos, aliases, junctions, symlinks, reparse points e hard links nos arquivos de logging. Falhas de criação/abertura/flush produzem erro não zero e unwinding RAII.

Cada ancestral do DataRoot é aberto sem seguir reparse points, inspecionado por GetFileInformationByHandleEx/GetFileInformationByHandle e comparado com GetFinalPathNameByHandleW. Handles sem FILE_SHARE_DELETE permanecem vivos; filhos duplicam esses handles. O destino final completo deve corresponder ao caminho esperado, sem decisão baseada apenas em prefixos. Arquivos usam OPEN_ALWAYS/FILE_FLAG_OPEN_REPARSE_POINT e são inspecionados antes de qualquer truncation/write; nNumberOfLinks deve ser 1. A criação dos diretórios novos começa somente abaixo da Known Folder. O logging usa nomes fixos e slots limitados.

Não se promete resistência absoluta a um processo externo hostil com privilégios equivalentes ou superiores que altere simultaneamente o filesystem. Também não se promete neutralizar drivers, injeção de código ou modificações do sistema operacional. Esses limites não relaxam a rejeição dos inputs e fixtures controlados pelo produto.

Evidência: I-3 rejeita entradas sintéticas proibidas; I-4 constrói junction, symlink e hard link em uma fixture isolada, comprova rejeição e verifica marker intacto. Nenhum teste utiliza caminho operacional real. I-7 verifica o limite de dois slots de 1024 bytes sob sua fixture. I-2 verifica configuração inválida sem criação de runtime. Resultados em `sentrium-final/*-test.log` e `sentrium-reproduction/*-test.log`.

As fixtures de alias usam destinos controlados sob a própria Foundation, evitando escrever em uma árvore externa para demonstrar rejeição. A implementação rejeita qualquer reparse point, inclusive aquele cujo destino ainda seria interno. Nenhum ensaio com adversário externo concorrente foi executado ou exigido.
