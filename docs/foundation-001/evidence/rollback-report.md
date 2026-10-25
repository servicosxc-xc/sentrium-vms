# SENTRIUM — rollback demonstrado

Resultado: **PASS**, dentro da execução isolada descrita em rollback-result.json.

Foi executado o binário Release com `--health --test-run <UUID>`. O processo retornou 0; o diagnóstico registrou Workers=0 e o último evento de log foi shutdown-completed. Após encerramento, os caminhos absolutos da fixture e do destino foram conferidos contra as raízes autorizadas. A fixture não continha reparse points.

Somente `DataRoot/tests/<UUID>` foi movido para `docs/foundation-001/evidence/sentrium-rollback-<UUID>`. O diretório de origem deixou de existir; nomes, tamanhos e SHA-256 dos arquivos arquivados coincidiram com o inventário anterior. Zero processos sentrium-server, sentrium-unit-tests ou sentrium-integration-tests remanescentes. Os logs foram preservados. Nenhuma ferramenta global foi desinstalada.

Os builds anteriores também foram arquivados dentro de `out/`: sentrium-initial-debug conserva a primeira validação; sentrium-validated-debug/release conservam o ciclo aprovado antes da reprodução. Os builds atuais permanecem em sentrium-debug/release. Nenhum arquivo legado foi envolvido.

Para arquivar integralmente esta Foundation: aguardar todos os executáveis encerrarem, preservar os relatórios, conferir a identidade da raiz do workspace e do DataRoot exclusivo e arquivar essas duas árvores sem seguir reparse points. Não remover `%LOCALAPPDATA%/Sentrium`, outras Foundations, `_toolchain` nem componentes globais. A demonstração isolada foi escolhida para preservar a auditoria; a remoção integral dos artefatos entregues não foi executada.
