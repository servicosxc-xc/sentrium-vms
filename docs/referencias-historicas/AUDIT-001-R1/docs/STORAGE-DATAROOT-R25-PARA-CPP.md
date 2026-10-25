# SQLite/DataRoot/segurança — r25 → C++

## Método

Schema obtido do SQL no código, não de consulta ao banco operacional. Nenhum banco foi aberto nesta auditoria. [DDL estruturado](../artifacts/SQLITE-SCHEMA-R25.json). SQLite/WAL permanece; não criar ORM pesado nem migrar dados reais nesta fase.

## Mapa exato do schema inicial

| Objeto r25 | Colunas / chave | C++ inicial |
|---|---|---|
| records | category TEXT NOT NULL, id TEXT NOT NULL, occurred INTEGER NOT NULL, owner TEXT NOT NULL default'', search TEXT NOT NULL default'', payload TEXT NOT NULL; PK(category,id) | manter nomes/tipos/PK e semântica de JSON/categoria |
| audit | id INTEGER PK, occurred INTEGER NOT NULL, action TEXT NOT NULL, detail TEXT NOT NULL | manter; não armazenar segredo em detail |
| integration_outbox | id TEXT PK, created INTEGER NOT NULL, state TEXT NOT NULL default'Blocked', payload TEXT NOT NULL | manter estados e payload; defaults da tabela não significam que produtor r25 gere Blocked |
| evidence_gateway | id TEXT PK, evidence_id TEXT NOT NULL, plate_crop INTEGER NOT NULL, expires INTEGER NOT NULL | manter id externo opaco e expiry UTCms |
| integration_evidence | event_id TEXT PK, evidence_id TEXT NOT NULL, protect_until INTEGER NOT NULL | manter vínculo/pin; atualmente sem FK declarada |

Índices: ix_records_time(category,occurred DESC,id); ix_records_owner(category,owner,occurred DESC,id); ix_records_media_type(category,json_extract(payload,'$.eventType'),occurred DESC,id); ix_records_media_camera(category,owner,json_extract(payload,'$.channelIndex'),occurred DESC,id); ix_gateway_expiry(expires); ix_gateway_evidence(evidence_id,plate_crop,expires); ix_integration_evidence(evidence_id,protect_until); ix_outbox_state(state,created), este último criado no store Outbox. Cinco tabelas/oito índices explícitos além de autoíndices de PK; não inventar tabelas separadas para cada DTO no schema inicial.

user_version=1, journal_mode=WAL, synchronous=FULL, busy_timeout10000ms, foreign_keys=ON (não implica que existam constraints FK nos vínculos). Uma conexão por operação, Pooling=false em .NET. Versão>1 é recusada pelo código atual. occurred/created/expires/protect_until são UTC Unix ms; payloads usam JSON web/camelCase e DateTimeOffset serializado, não SQL datetime textual uniforme.

Categorias: lpr.readings; lpr.reviews.latest/history; muralha.history; events; evidence; evidence.paths; settings. settings inclui muralha.config/mappings, lpr.master, storage.retention e storage.metrics. Perfis/regras/devices ainda são JSON externos ao records, não migrados integralmente para tabelas. Metadados/paths são internos; APIs públicas não podem simplesmente expor payload evidence.paths.

LocalDatabase.Put usa INSERT ... ON CONFLICT UPDATE ou INSERT OR IGNORE; Page ordena occurred DESC/id DESC com LIMIT/OFFSET, filtros owner/search e clamp de página/tamanho; imports JSONL em transação são idempotentes por ID, ignoram inválidos preservando arquivos; backup usa API SQLite. Distinguir linhas válidas de linhas realmente novas inseridas. Catálogo indexa arquivos em background, refresh de clip usa fila limitada, thumbs gerados sob demanda. Não reproduzir scan de filesystem por request de mídia.

## Wrapper C++ RAII

Connection/Statement/Transaction únicos, sqlite3_close_v2/finalize/rollback em destructors noexcept; bind tipado e checar todos return codes/extended errors. Statement reset/clear bindings por execução, não compartilhar entre threads. Writer dedicado por store com fila bounded/deadlines e ack de commit; leitores por pool limitado/read-only. JSON1 disponível e versão SQLite compatível com índices por expressão; pin de compilação/hashes e testes de query plans. WAL exige gestão de checkpoint/reader lifetime; não copiar só db enquanto há WAL ativo. Check/integrity e backup só via ferramenta explícita, não em hot path/health. Backup de DB não restaura mídia apagada.

Fonte r25 não se transforma em fixture pelo uso de `immutable=1` no banco vivo: ignoraria WAL e pode dar leitura inconsistente. Desenvolvimento usa fixture sintética ou cópia consistente/sanitizada preparada posteriormente com autorização. Cada oracle/C++ usa cópia independente. Primeiro abrir read-only a cópia, conferir user_version/schema/categorias/índices/JSON, depois habilitar writer só no root v2-dev. Migração futura é versionada/transacional, backup validado e ledger de versão; não mudar user_version em produção na AUDIT.

## DataRoot e arquivos atuais ou possíveis

| Local | Conteúdo / situação |
|---|---|
| raiz DataRoot | xc-videoia.sqlite3 + -wal/-shm; devices.json protegido, ai-rules.json, lpr-profiles.json |
| security/dp-keys | keyring ASP.NET Data Protection; DPAPI máquina no Windows; não copiar para pacote/auditoria |
| lpr/ | readings.jsonl, attempts/reviews JSONL e dados legados importáveis; SQLite autoritativo conforme store |
| evidence/por data | JPEG novo/BMP legado, DAV clips, JSON metadata, plate.jpg/plate.bmp; paths internos no catálogo |
| evidence/thumbnails | cache derivado JPEG até192px, fingerprint; cache não é fonte de verdade |
| lpr-dataset/manual | exemplos/ground truth manuais, não limpar como rejeição automática |
| lpr-dataset/automatic/rejected | diagnóstico de rejeição/variantes/JSON/BMP por política própria |
| integrations/muralha-paulista | configuração/histórico legados/espelhos JSON/JSONL; registros/mappings atuais no SQLite |
| root/logs | proposta de organização v2, não log central totalmente implementado em r25 |
| bin/.../sdk_log | log SDK observado aberto na r25; fora do DataRoot central, pode mudar por processo já aberto |
| %LOCALAPPDATA%/XCVideoIA | client-preferences.json e last-visualization.json, fora do root Server |
| %LOCALAPPDATA%/XC Video IA/MediaCache | DAV baixado pelo Client, nome legado com espaços; cache sem política completa |
| temp/XC.VideoIA/lpr-bridge | BMPs temporários da bridge, removidos best-effort; não é evidência autoritativa |
| .xc-videoia/lpr/venv e caches do pacote | dependências/modelos Python externos; não supor que estão no DataRoot nem consultar cache operacional nesta auditoria |

Resolução atual: XC_DATA_ROOT > Storage:DataRoot > ContentRoot/data; run-xc usa LocalAppData/XCVideoIA/data. `RequestedDataRoot` do Client registra preferência, não move raiz nem valida migração. Caminhos possíveis inferidos de código, não inventário do conteúdo operacional; nenhum diretório operacional foi enumerado.

Root v2 obrigatório no desenvolvimento: **%LOCALAPPDATA%/XCVideoIA-v2-dev**. RootContext explícito/instância e denylist dos roots r25/r24/produção, canonicalização considerando junction/symlink e comparação de identidade de volume/arquivo quando necessário. Tools de writer/migrate/restore exigem root v2-dev e guard de única instância; impedir aliases de caminho que contornem o guard. Logs/dumps/diagnósticos/artefatos também ficam fora de v1. Nenhum root v2-dev foi criado nesta auditoria.

## Retenção e Gateway

Retenção padrão desativada, background após30s/a cada minuto, até100 elegíveis; idade/bytes/free-space e proteção de recentes1h/reviews. Rejected automático tem regra separada, manual não é removido. Paths fora da área gerenciada/reparse são bloqueados. Metadados/readings/audit são mantidos. Links Gateway válidos, Outbox Pending/Sending/Unknown e pins recentes bloqueiam delete; seleção exclui protegidos antes de LIMIT e revalida sob fence. Quantidade de mídia usada vem do catálogo, não total do volume. Preservar coerência catálogo-arquivo e auditar falha parcial; root v1 nunca deve ser alvo do writer C++.

Rollback código r24 não entende pins r25 e exige retenção desativada enquanto esses vínculos forem relevantes. Backup só de banco não recupera imagens. Inicializar dois Servers no mesmo root não é modo suportado.

## Credenciais e equivalente Windows

enc:v1 contém **envelope ASP.NET Data Protection**, purpose XC.VideoIA.DeviceSecrets.v1 e ApplicationName XCVideoIA. As chaves do keyring são protegidas por DPAPI local-machine; não é simplesmente CryptProtectData aplicado à senha. Portar formato requer implementação oficial do envelope ou migrador .NET isolado em etapa autorizada/máquina adequada; alternativa é reprovisionar credenciais. Não prometer desencriptar tudo em C++ chamando CryptUnprotectData no blob.

Formato novo versionado com DPAPI/CNG conforme threat model, key ACL, identidade do serviço/máquina e auditoria. Secret buffers de ownership exclusivo, tamanho limitado e SecureZeroMemory quando possível; strings/temporários/cópias em libs/OS podem permanecer, então não prometer secure erase universal. Tokens/Authorization/senhas não entram em logs/URLs/dumps de diagnóstico; export redaction é allowlist de campos. Kestrel/TLS/certificados atuais passam a backend TLS/cert store/proxy explícito, nunca confiar forwarded headers sem lista de proxies.

## Backlog de schema, não aplicado

Listas LPR: plate_lists(id,name,kind,policy), plate_entities(id,canonical_plate,owner_ref,vehicle_metadata), list_memberships(list_id,plate_id,valid_from,expires,observations), list_action_rules e audit com actor. Constraints de membership/lookup por placa normalizada e precedência entre listas precisam de ADR e revisão; eventos mantêm plate+EventId e snapshot da classificação para histórico. Quick add/remove/move/expiry/black-whitelist/custom/ações condicionais são requisitos, ainda não runtime r25.

Users/roles/permissions/role_permissions, device/LPR scopes e actor em audit serão adicionados em migração futura separada. Admin/Operator/Viewer/Configurator são papéis propostos, não contas já existentes. Não acoplar lista/autorização ao schema JSON v1 por campos anônimos improvisados.

Referências: [SQLite public domain](https://www.sqlite.org/copyright.html), [backup API](https://www.sqlite.org/c3ref/backup_finish.html). A compatibilidade r25 acima foi derivada do código, não de uma conexão ao banco.