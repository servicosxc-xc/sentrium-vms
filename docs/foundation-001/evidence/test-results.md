# SENTRIUM — builds e testes

Identidade das fontes: `source-b8b6d42374064dc8`; versão `0.0.0-foundation001`. Não há commit Git. O identificador deriva dos hashes dos arquivos de fonte/build/scripts, preservados em source-inventory.json. Documentos de resultado não alteram essa identidade.

| Execução | Configuração | Unitários | Integração | Falhas | Warnings |
|---|---|---|---|---|---|
| sentrium-final | Debug | 8/8 | 11/11 | 0 | 0 |
| sentrium-final | Release | 8/8 | 11/11 | 0 | 0 |
| sentrium-reproduction | Debug, novo diretório vazio | 8/8 | 11/11 | 0 | 0 |
| sentrium-reproduction | Release, novo diretório vazio | 8/8 | 11/11 | 0 | 0 |

Todos os configure/build/test finais retornaram 0. Política `/W4 /WX` para os cinco targets; não houve supressão global de diagnósticos. Os logs verbosos incluem headers/toolchain efetivos. A reprodução comprova igualdade de identidade de fontes, lock e resultados lógicos; não declara PE/PDB bit a bit idênticos, pois o link não foi configurado como build binariamente determinístico.

| ID documental (CTest) | Comportamento verificado |
|---|---|
| U-01 (U-1) | Estados/health e rejeição de transições inválidas |
| U-02 (U-2) | Limites tipados de configuração |
| U-03 (U-3) | Relógio injetável, startup/shutdown/uptime exatos |
| U-04 (U-4) | Ownership exclusivo, destruição única e unwinding |
| U-05 (U-5) | 100 cancelamentos/join determinísticos |
| U-06 (U-6) | Propagação de código de erro e falha de bootstrap |
| U-07 (U-7) | UTC, campos obrigatórios, escaping e linha única |
| U-08 (U-8) | Versão/build/configuração e null para métrica indisponível |
| I-01 (I-1) | Processo real, bootstrap e ordem do shutdown nos logs |
| I-02 (I-2) | Configuração inválida, erro 2 e nenhum runtime criado |
| I-03 (I-3) | Root autorizado, traversal/UNC/device/nomes proibidos |
| I-04 (I-4) | Junction, symlink e hard link reais; marker preservado |
| I-05 (I-5) | Cancelamento de atividade cooperativa |
| I-06 (I-6) | Handles sem vazamento e processo filho encerrado |
| I-07 (I-7) | Logging em fixture validada, rotação e limites |
| I-08 (I-8) | Duas execuções com mesmos campos lógicos; tempos variáveis explícitos |
| I-09 (I-9) | Parser de importações do executável real e máquina AMD64 |
| I-10 (I-10) | Health/erro CLI, privacidade e exclusão mútua do runtime |
| I-11 (I-11) | Versão/build nos logs e diagnóstico |

Evidências brutas: test-summary.json, quatro relatórios JUnit, quatro LastTest.log, logs completos, comandos JSON e exit codes nas duas pastas de execução. A primeira tentativa teve um erro do script antes do configure e a primeira validação Debug teve falso positivo de classificação do runtime atomic-wait. Ambos foram preservados em sentrium-initial-automation-error.txt e sentrium-validation-classification.md, junto com o JUnit inicial reprovado. Não houve mudança do requisito nem teste desativado.

Sanitizador: instalação aprovada sem bibliotecas/runtime ASan. Avaliação em sanitizer-evaluation.json; nenhuma execução de ASan é declarada. A revisão humana do ADR-001 deve considerar essa pendência.
