# Dependências e licenças

Revisão de fontes primárias em 23/10/2026; não substitui aprovação jurídica. Versões abaixo são candidatas, não lockfile aprovado. Binário, configuração de build, pesos, drivers e redistribuição precisam SBOM, hash e termos próprios. Não foram baixadas dependências nesta auditoria.

| Dependência | Versão/base candidata | Licença/uso | Gate de distribuição |
|---|---|---|---|
| Qt | 6.12 LTS | comercial ou LGPLv3 nos módulos elegíveis | selecionar módulos; obrigações de substituição/relink, avisos, fontes de alterações Qt; static exige plano específico |
| Qt módulos GPL | evitar no produto proprietário sem contrato adequado | exemplos: Quick3D, Graphs, MQTT, HTTPServer, VirtualKeyboard | não presumir cobertos por LGPL; conferir lista por versão |
| FFmpeg | 9.0.2 candidata | núcleo majoritariamente LGPL2.1+; build muda licença | registrar configure; evitar enable-gpl/nonfree por padrão; patentes separadas |
| Drogon | 1.9.14 candidata | MIT | avisos; auditar Trantor, TLS e transitivas |
| OpenSSL | versão compatível fixar | licença depende versão | não reutilizar automaticamente libeay32/ssleay32 legados do SDK como TLS do servidor |
| ONNX Runtime | r25 1.22.1/1.24.4 em diferentes caminhos; v2 fixar uma versão compatível | MIT | providers, CUDA/TensorRT/DirectML têm dependências/termos adicionais |
| CUDA/NVDEC/TensorRT | matriz driver/SDK/modelo a fixar | termos NVIDIA | EULA, DLLs redistribuíveis, GPU/driver e caching por hardware |
| DirectML/WinML | versão Windows/SDK a fixar | componentes Microsoft, termos próprios | não inferir redistribuição de WinML a partir de MIT do ORT |
| SQLite | stable fixar em baseline | domínio público | extensão/criptografia comercial não incluída |
| JsonCpp | stable fixar | domínio público ou MIT | recomendado na fronteira Drogon; notices conforme opção |
| nlohmann/json | alternativa, não segundo DOM por padrão | MIT | só adicionar com benefício e conversões medidas |
| simdjson | adiar até profiling | Apache2 | avisos e patentes conforme licença |
| spdlog / fmt | versões compatíveis fixar | MIT | evitar dois fmt incompatíveis e registrar opção externa/bundled |
| OpenCV 4.x | subconjunto mínimo | Apache2 no núcleo atual | verificar codecs/third-party; não empacotar módulos sem uso |
| GoogleTest | stable fixar | BSD3 | ferramenta de testes, avisos |
| Google Benchmark | stable fixar | Apache2 | ferramenta de benchmark, avisos |
| fast-alpr / fast-plate-ocr / open-image-models | instalados 0.4.0 / 1.1.0 / 0.6.0 | MIT de código | referência comportamental; licença dos pesos é independente |
| YOLOX | modelo r25 identificado por hash | projeto Apache2 | confirmar proveniência/licença específica do peso distribuído |
| Intelbras NetSDK/PlaySDK/P2P | kit C++ General_NetSDK_3.050_PlaySDK_3.042 localizado; libt2u nativa identificada | proprietária/desconhecida neste material | disponibilidade .h/.lib resolvida; EULA/redistribuição libt2u, standalone/bootstrap oficiais, firmware e compatibilidade permanecem gates |
| WinUI / Windows App SDK | estável a fixar, evitar preview implícito | WinUI MIT; runtime com termos próprios | validar requisitos OS/instalador e redistribuição |
| CMake/Ninja/vcpkg/MSVC | C++20, VS2022 v143 propostos | licenças de ferramentas distintas | baseline vcpkg/registro/hash; Build Tools e SDK conforme contrato |

LGPL não se resolve apenas com link dinâmico: usuário precisa exercer direitos aplicáveis, incluindo substituir/relinkar biblioteca; EULA não pode anular direitos pertinentes de engenharia reversa. Confirmar obrigações, fontes, notices e instalação com jurídico. Comercial pode simplificar algumas obrigações, mas termos dependem contrato. GPL/nonfree e patentes de codecs são análises separadas.

## Fontes primárias

- [Qt licenças e módulos](https://doc.qt.io/qt-6/licensing.html), [obrigações LGPL](https://www.qt.io/development/open-source-lgpl-obligations).
- [FFmpeg releases](https://ffmpeg.org/download.html), [licença/configurações](https://www.ffmpeg.org/legal.html).
- [Drogon](https://github.com/drogonframework/drogon), [MIT](https://raw.githubusercontent.com/drogonframework/drogon/master/LICENSE).
- [ORT MIT](https://raw.githubusercontent.com/microsoft/onnxruntime/main/LICENSE), [DirectML em sustentação](https://onnxruntime.ai/docs/execution-providers/DirectML-ExecutionProvider.html), [WinML](https://learn.microsoft.com/en-us/windows/ai/new-windows-ml/overview).
- [NVDEC](https://docs.nvidia.com/video-technologies/video-codec-sdk/13.0/nvdec-video-decoder-api-prog-guide/index.html).
- [SQLite](https://www.sqlite.org/copyright.html), [JsonCpp](https://raw.githubusercontent.com/open-source-parsers/jsoncpp/master/LICENSE), [nlohmann](https://raw.githubusercontent.com/nlohmann/json/develop/LICENSE.MIT), [simdjson](https://raw.githubusercontent.com/simdjson/simdjson/master/LICENSE).
- [spdlog](https://raw.githubusercontent.com/gabime/spdlog/v1.x/LICENSE), [fmt](https://raw.githubusercontent.com/fmtlib/fmt/master/LICENSE), [OpenCV](https://raw.githubusercontent.com/opencv/opencv/4.x/LICENSE).
- [GoogleTest](https://raw.githubusercontent.com/google/googletest/main/LICENSE), [Benchmark](https://raw.githubusercontent.com/google/benchmark/main/LICENSE).
- [fast-alpr](https://raw.githubusercontent.com/ankandrew/fast-alpr/master/LICENSE), [OCR](https://raw.githubusercontent.com/ankandrew/fast-plate-ocr/master/LICENSE), [open-image-models](https://raw.githubusercontent.com/ankandrew/open-image-models/master/LICENSE), [YOLOX](https://raw.githubusercontent.com/Megvii-BaseDetection/YOLOX/main/LICENSE).
- [WinUI](https://raw.githubusercontent.com/microsoft/microsoft-ui-xaml/main/LICENSE), [vcpkg manifest](https://learn.microsoft.com/en-us/vcpkg/concepts/manifest-mode).

Revisão documental Intelbras de 23/10/2026: [addendum Intelbras/Cloud](../AUDIT-001-INTELBRAS-CLOUD-ADDENDUM.md). Preservados demais resultados.