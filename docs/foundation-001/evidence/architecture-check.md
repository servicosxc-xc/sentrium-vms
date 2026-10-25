# SENTRIUM — arquitetura e binários

`sentrium-server → sentrium-platform-win → sentrium-core`. Os dois testes dependem respectivamente do core e da plataforma. Não há dependência reversa core/platform, frameworks de mídia, UI ou bibliotecas externas. Targets mínimos: duas static libraries e três executáveis. O core usa apenas a biblioteca padrão C++20; `Windows.h` e Known Folder/handles pertencem à plataforma.

Lifecycle expõe Starting, Ready, Stopping, Stopped e Failed. `stopping()` fecha a admissão antes do registro/cancelamento. Activity possui um único std::jthread, recebe std::stop_token, sincroniza inicialização com latch e encerra por condition_variable_any stop-aware; join antecede a liberação. Destrutor repete stop de forma idempotente. Recursos Windows usam um Handle exclusivamente movível; objetos movidos ficam inválidos. Falhas de bootstrap desenrolam os recursos em RAII e retornam códigos tipados.

U-1, U-4, U-5, U-6 e I-1/I-5/I-6 exercitam esses comportamentos. I-6 mede a contagem de handles antes/depois e aguarda o processo filho. Não é usado sleep para aprovar testes. O health é um snapshot diagnóstico, entregue após shutdown limpo, e não um serviço persistente.

Arquitetura AMD64 verificada por parser PE em I-9 e dumpbin /headers para Debug e Release. Consulte `sentrium-final/sentrium-debug-headers.log`, `sentrium-final/sentrium-release-headers.log` e os arquivos equivalentes em `sentrium-reproduction`. O compilador e linker efetivos foram fixados por caminho e registrados no PRE-FLIGHT.
