# SENTRIUM — matriz AC-01 a AC-26

Base: ordem FINAL R1, SCOPE 1.1 informado como APPROVED, F001 Decision Gate AUTHORIZED. Build `source-b8b6d42374064dc8`. Os caminhos abaixo são relativos a esta pasta de evidências. PASS refere-se ao critério e ao escopo descrito, não à aceitação dos ADRs.

| ID | Resultado | Teste / método | Arquivo de evidência | Observações | Limitações |
|---|---|---|---|---|---|
| AC-01 | PASS | vswhere, versões, SDK, HEAD/tag, ambiente oficial | preflight-report.md; sentrium-final/preflight.json; sentrium-final/toolchain-hashes.json | Lock R1 integral, x64/x64 | Revisão SDK e diretórios têm formatos distintos |
| AC-02 | PASS | CMake/Ninja em diretórios vazios | sentrium-final/sentrium-debug-configure.log; sentrium-final/sentrium-release-configure.log | Windows x64 confirmado | Ambiente Windows inspecionado nesta máquina |
| AC-03 | PASS | Build Debug do zero, exit 0 | sentrium-final/sentrium-debug-build.log; sentrium-final/sentrium-debug-build-exit.txt | /MDd, /W4 /WX | Debug exige CRT de desenvolvimento |
| AC-04 | PASS | Build Release independente, exit 0 | sentrium-final/sentrium-release-build.log; sentrium-final/sentrium-release-build-exit.txt | /MD, cache/output separados | Sem distribuição comercial |
| AC-05 | PASS | U-01 a U-08 em ambas configurações | test-results.md; sentrium-final/*-junit.xml | 8/8 por configuração | Testes mínimos Foundation-001 |
| AC-06 | PASS | I-01 a I-11 em ambas configurações | test-results.md; sentrium-final/*-junit.xml | 11/11 por configuração | Symlink requer capacidade Windows existente |
| AC-07 | PASS | Parser PE I-9 e dumpbin /headers | sentrium-final/sentrium-debug-headers.log; sentrium-final/sentrium-release-headers.log | Machine 8664 AMD64 | Não comprova outras arquiteturas |
| AC-08 | PASS | Importações reais e revisão de fontes | dependency-inventory.md; sentrium-final/*-imports.log | Sem SIM Next ou carregamento dinâmico próprio | Dependências internas normais do OS documentadas |
| AC-09 | PASS | I-9, allowlist runtime e dumpbin | sentrium-final/*-test.log; sentrium-final/*-imports.log | Nenhuma DLL Intelbras | Limitado aos artefatos produzidos |
| AC-10 | PASS | Fontes, manifest vazio, importações | sentrium-final/source-inventory.json; dependency-inventory.md | Sem modelo ou loader IA/LPR | Não enumera instalações alheias |
| AC-11 | PASS | Auditoria dos pontos de acesso e fixtures | filesystem-process-api-audit.txt; dataroot-security.md | Nenhum DataRoot legado lido/enumerado | Não utiliza raízes reais em testes negativos |
| AC-12 | PASS | Known Folder + root fixo, I-3 e health real | dataroot-security.md; health-output.json; rollback-result.json | Runtime e testes isolados | Diretórios ancestrais próprios criados quando necessários |
| AC-13 | PASS | I-3/I-4, handles finais, markers intactos | threat-model.md; sentrium-final/*-test.log | Junction/symlink/hard link rejeitados | Adversário externo privilegiado concorrente fora do threat model |
| AC-14 | PASS | Processo local --health retorna e encerra | health-output.json; health-exit.txt; sentrium-final/*-test.log | Diagnóstico JSON sem caminhos privados | Snapshot de Ready, não monitoramento de outro processo |
| AC-15 | PASS | Revisão das APIs de produto/importações | negative-source-audit.json; sentrium-final/*-imports.log | Sem código de listener, socket, pipe ou transporte IPC | Lock de arquivo é exclusão mútua; não há tracing global do OS |
| AC-16 | PASS | I-1/I-5 e log ordenado | health-runtime.jsonl; sentrium-final/*-test.log | Cancelamento, join, liberação, logging finalizado | Timeout forçado existe apenas no harness e não ocorreu |
| AC-17 | PASS | I-6 e consulta após rollback | rollback-result.json; sentrium-final/*-test.log | Workers=0, handles estáveis, nenhum processo residual | Observações das execuções controladas |
| AC-18 | PASS | U-8/I-11 e log real | health-runtime.jsonl; health-output.json | Versão, BuildId, RunId e UTC | BuildId de hash de fontes; não há commit Git |
| AC-19 | PASS | FakeClock U-3, latch/join, I-8 | test-results.md; sentrium-final/*-test.log | Asserts lógicos não dependem de sleeps | Tempos/UUID reais variam e não são comparados literalmente |
| AC-20 | PASS | Segundo ciclo do zero, mesmos hashes de fonte | test-summary.json; sentrium-reproduction/source-inventory.json; sentrium-reproduction/*-test.log | 19/19 Debug e Release novamente | Reprodução lógica/toolchain, não PE/PDB bit a bit |
| AC-21 | PASS | Inventários de toolchain/runtime/importações | dependency-inventory.md; runtime-inventory.json; sentrium-final/preflight.json | Ferramentas separadas do runtime | Candidatos de resolução do runtime, não tracing de módulos |
| AC-22 | PASS | /W4 /WX e varredura dos quatro logs aprovados | warnings-check.json; sentrium-final/*-build.log | Zero warnings detectados; sem suppressions | Logs brutos preservados para revisão |
| AC-23 | PASS | Evidências iniciais C++20/lifetime/cancelamento | adr-001-evidence.md; sanitizer-evaluation.json | ADR-001 continua Proposed | ASan indisponível; execução instrumentada pendente |
| AC-24 | PASS | Ambiente, presets, quatro builds/testes e SBOM | adr-002-evidence.md; sentrium-final/; sentrium-reproduction/ | ADR-002 continua Proposed | Não constitui aceitação automática |
| AC-25 | PASS | Inventário proporcional com origens/hashes/licenças conhecidas | sbom-initial.md; runtime-inventory.json; tool-license-inventory.json; auxiliary-tool-inventory.json | Sem bibliotecas externas de produto | Licenças não verificadas marcadas NOASSERTION; OS interno não exaustivo |
| AC-26 | PASS | Arquivamento isolado após shutdown, comparação SHA-256 | rollback-report.md; rollback-result.json | Origem ausente, arquivo íntegro, evidência preservada | Não removeu a entrega completa nem ferramentas globais |

Nenhum critério obrigatório permanece FAIL, BLOCKED ou NOT VERIFIED no escopo acima. A execução de sanitizador permanece uma limitação explícita da evidência inicial ADR-001; não é apresentada como teste executado ou como aceitação desse ADR. Falhas iniciais de automação/classificação foram preservadas e corrigidas antes dos dois ciclos finais, sem desativar testes.
