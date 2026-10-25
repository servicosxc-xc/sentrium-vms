# Acervo histórico — complemento e pendências (25/10/2026)

Este registro complementa `PRESERVACAO-E-PENDENCIAS.md` sem editar documentos anteriores. Todo o acervo permanece **REFERENCE ONLY**, não é parte dos targets de build.

## Documentos adicionais recuperados nesta revisão

| Arquivo | Verificação |
|---|---|
| [INVENTARIO-R25.json](AUDIT-001-R1/artifacts/INVENTARIO-R25.json) | JSON íntegro sintaticamente após reconstrução textual; 377 fontes identificadas, 1922 arquivos totais reportados |
| [MATRIZ-PARIDADE.json](AUDIT-001-R1/artifacts/MATRIZ-PARIDADE.json) | JSON íntegro sintaticamente após reconstrução textual; 377 componentes |
| [Checksum do ZIP de auditoria](AUDIT-001-R1/artifacts/XC-VideoIA-v2-CPP-AUDIT-001.zip.sha256) | Conteúdo idêntico à segunda cópia disponível; checksum é histórico, não recalculado no ZIP por esta operação |
| [Handoff inicial SENTRIUM](HANDOFF-INICIAL-SENTRIUM-VMS.md) | Documento inicial, referência de intenção e princípios do produto |
| [Context Manifest 1.0 histórico](SENTRIUM-VMS-CONTEXT-MANIFEST-1.0-HISTORICO.txt) | Snapshot documental histórico; estados mencionados nele não substituem as decisões posteriores de Foundations e ADRs |

Os dois grandes JSONs foram reconstruídos integralmente pelo conteúdo textual disponibilizado na plataforma e validados com parser JSON. **Não foi comprovada equivalência byte a byte com os originais** (espaçamento, line endings, codificação e checksum de origem podem diferir).

## Fora do acervo Git / ainda não recuperados em forma publicável

| Material | Disponibilidade conhecida | Motivo do não envio |
|---|---|---|
| `XC-VideoIA-v2-CPP-AUDIT-001 (2)(1).zip` (~1,36 MB) | anexo disponível, mas ZIP original binário não transferido para Git | não foi possível efetuar transferência binária verificável pelos conectores nesta sessão; preservar original e validar SHA256 |
| Outras variantes `XC-VideoIA-v2-CPP-AUDIT-001(2).zip` e `(3).zip` (~671 KB cada) | anexos disponíveis | variantes não provadas como equivalentes ao pacote canônico; evitar duplicações/atribuição errada de hashes |
| `XC-VideoIA-v1-Foundation-007-r25(2).zip` (~33 MB) | anexo disponível | contém código/artefatos mistos; publicar só após inspeção de dados, credenciais, binários e licenças |
| `General_NetSDK_3.050_PlaySDK_3.042 (1)(1).zip` (~447 MB) | anexo disponível | tamanho acima do limite normal do GitHub por arquivo; licenciamento/redistribuição de terceiros pendentes |
| Capturas RAW/DHAV, logs de benchmarks, PCAPs e registros de laboratório locais | não recuperados como bytes neste fluxo | precisam ser disponibilizados separadamente, com preservação de proveniência e revisão de dados sensíveis |
| Artefatos internos do ZIP da auditoria como `R25-SHA256-ANTES.json`, `R25-SHA256-DEPOIS.json` e outros JSONs não anexados separadamente | não extraídos do ZIP neste fluxo | dependem da extração autenticada do pacote original |

## Duplicatas disponíveis e não replicadas

Segundas cópias de `AUDIT-001-RESULTADO.md`, `AUDIT-001-INTELBRAS-CLOUD-ADDENDUM.md`, `MAPA-TESTES-R25-PARA-CPP.md` e `.sha256` foram comparadas textualmente e apresentaram conteúdo idêntico às selecionadas. Cópias repetidas de outros Markdown não foram incorporadas novamente, para evitar ruído.

## Política de continuidade

1. Nenhum arquivo pré-existente no repositório foi alterado nesta inclusão.
2. Não alterar `main` ou a tag histórica sem revisão humana do pull request.
3. Não promover ADRs históricos arquivados a decisões vigentes.
4. Não considerar os ZIPs legados como dependências de compilação do SENTRIUM.
5. Manter binários/SDKs e capturas sensíveis em armazenamento privado adequado com SHA256, até avaliação separada.
