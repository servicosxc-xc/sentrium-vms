# Auditoria r25 → C++ — AUDIT-001

23/10/2026. Escopo exclusivo de inventário, leitura de código e propostas. Nenhum código C++, produto, build, teste do produto, migração ou acesso ao DataRoot operacional foi executado. A r25 é oracle/rollback, não template para tradução arquivo a arquivo.

## Base verificável

Manifesto inicial completo: **1.922 arquivos / 723.275.780 bytes**, incluindo bin/obj/artefatos/logs; hashes em [R25-SHA256-ANTES.json](../artifacts/R25-SHA256-ANTES.json). Inventário de fontes/recursos: **377 arquivos** excluindo bin/obj/artifacts/data, ZIP e SHA externos. Escopos diferentes explicam a diferença. Código C#: **63.446 linhas físicas**, das quais 36.043 são os três arquivos de wrapper NetSDKCS; sem wrapper: **27.403**. Linhas não representam esforço de portabilidade nem cobertura semântica. As contagens de símbolos/locks/tasks são lexicais, não AST; comentários e sobrecargas exigem leitura contextual.

Há **13 projetos: oito de produto, três labs, dois de testes**. Foram encontrados **242 P/Invoke**, incluindo repetição entre produção/labs; apenas **10 métodos NETClient** são chamados fora do wrapper. Há 15 DLLs x64 e um ONNX YOLOX no pacote; nenhum header .h/import library .lib oficial. APIs: 75 registros de verbos HTTP, mais mídia WebSocket e SignalR. Os **151 casos aprovados** constam dos relatórios já existentes da r25; não foram reexecutados nesta auditoria.

| Módulo/projeto | Arquivos | Linhas código/markup | Todas as linhas textuais |
|---|---:|---:|---:|
| XC.VideoIA.AI | 7 | 859 | 871 |
| XC.VideoIA.Client | 82 | 14259 | 14297 |
| XC.VideoIA.Client.Tests | 4 | 227 | 231 |
| XC.VideoIA.Core | 9 | 309 | 316 |
| XC.VideoIA.DecoderRenderProbe | 10 | 664 | 685 |
| XC.VideoIA.Infrastructure.Intelbras | 23 | 37231 | 37252 |
| XC.VideoIA.LprEngine | 7 | 502 | 512 |
| XC.VideoIA.MediaTransportProbe | 2 | 290 | 301 |
| XC.VideoIA.MosaicRenderProbe | 14 | 1313 | 1334 |
| XC.VideoIA.Server | 55 | 8868 | 9033 |
| XC.VideoIA.SharedContracts | 22 | 1141 | 1148 |
| XC.VideoIA.Storage | 3 | 132 | 143 |
| XC.VideoIA.Tests | 9 | 1077 | 1095 |

Fontes de evidência: [inventário](../artifacts/INVENTARIO-R25.json), [matriz](MATRIZ-R25-PARA-CPP.md), [dependências](MAPA-DEPENDENCIAS.md), [testes](MAPA-TESTES-R25-PARA-CPP.md). Documentação histórica foi inventariada; decisões r25 prevalecem sobre READMEs antigos. Documento de validação antigo diz hardware pendente; o novo pedido fornece resultados reais posteriores. Ambos são preservados com proveniência distinta em [baseline](BASELINE-PERFORMANCE-R25.md).

## Achados prioritários

| ID | Achado e evidência | Implicação para v2 |
|---|---|---|
| F-001 | `IntelbrasCloudTunnelService.cs`: Assembly.LoadFrom/Reflection de SIMNext.P2PSDK, P2PAdapter, AddSDKPort/P2pIsActive/RelayConnection/DeleteSDKPort/Cleanup e libt2u externa | Revisado: libt2u nativa e wrapper P/Invoke confirmados pelas novas evidências; .NET não obrigatório. Gates IB-01..06 para produção, conforme addendum. |
| F-002 | `Native/README-LEGACY-SDK.md`: pacote CSharp64 V3.050...190304; 15 DLLs e milhares de declarações, sem .h/.lib | Kit externo General_NetSDK_3.050_PlaySDK_3.042 localizado com .h/.lib e exemplos x64; disponibilidade C++ resolvida. Validar compatibilidade/ABI/licenças; não inferir headers por exports. |
| F-003 | `IntelbrasDeviceService.cs:720`: PTZ chama NETClient.PTZControl diretamente; Runtime declara ser gateway único | Furo no isolamento/telemetria/serialização do SDK. Encapsular também PTZ no adapter nativo; não reproduzir o acoplamento. |
| F-004 | `AiEvidenceRecorder.cs:25–35`: ToArray do pacote e prebuffer por tempo em todo OnPacket, inclusive sessão só LPR; linhas 39–40 Write síncrono sob lock | Cópia contínua e memória dependente de bitrate persistem após r25; recording separado, packet refs e limites por bytes/idade. Custo real não isolado nesta auditoria. |
| F-005 | `CompressedMediaSubscription.TryPublish`: TryWrite em fila limitada, rejeita pacote se cheia | Não é drop seguro por frame/GOP. Definir resync/IDR/discontinuity ao perder comprimido; não aplicar latest-frame em bytes comprimidos. |
| F-006 | `PlaySdkDecoder.OnDraw`/VideoPipelineTelemetry: draw deduplicado por sequência de decode | Não mede apresentação DWM nem perda física. GPU ~9% agregada não prova engine decode/compute subutilizada. Instrumentar engines e presentation real. |
| F-007 | `MediaPlaybackWindow.xaml.cs`: GetByteArrayAsync inteiro, MediaCache sem quota explícita e sem cancelamento atrelado à janela | Redesenhar leitura em streaming/cache limitado/lifecycle cancelável. Não equivale a playback de gravações do DVR. |
| F-008 | `SecretProtector`: enc:v1: + ASP.NET Data Protection, purpose específico; keyring protegido por DPAPI | CryptUnprotectData direto no blob não é compatível. Reprovisionar ou migrador .NET isolado/consentido em etapa posterior. |
| F-009 | Dependências Python instaladas e worker: OCR uint8 NHWC por slots, não CTC genérico | Precisa portar contrato de cada modelo, configuração/alphabet/pad/region e variantes; chamar ONNX apenas não garante paridade. |
| F-010 | `RuntimeHub` usa SignalR; Drogon fornece WebSocket | Protocolo/negotiation/reconnect não são intercambiáveis. Adapter ou transporte v2 versionado com gate explícito. |
| F-011 | `EvidenceLifetimeFence`/retenção: coordenação apenas por processo; pin Pending/Unknown sem expiração automática | Manter uma instância por root; política de resolução/guardas de writer e quota devem nascer na v2. |
| F-012 | Tests r25 de fonte física/render utilizam planos/fakes/primitivas; LatestFrameSlot/RenderCadence não comandam PLAY_Play | Paridade exige 151 requisitos + novos testes nativos/hardware, sem afirmar correção de fluidez pelo unit test. |

## Portar, redesenhar, substituir

PORTAR: normalizador/consenso/dedup, geometria de ROI/gate/linha, occurrence policy, health/histerese, políticas de filas/recovery e estados de Master. Fazer primeiro testes golden contra r25.

REDESENHAR: media ownership/scheduler, workers de inferência, control plane HTTP, recording/evidence IO, persistência RAII, ciclo de vida e security adapter Windows. SUBSTITUIR: WPF por UI candidata, wrapper P/Invoke por headers oficiais, runtime Python/BMP/JSON por LPR C++ após equivalência. REUTILIZAR NATIVO: SDK licenciado/validado e fallback PlaySDK onde o formato exigir. MANTER COMPATIBILIDADE: REST/DTOs/JSON/SQLite/configurações/ações/evidências/Muralha. ADIAR: usuários completos, listas LPR, SMTP/relé/webhook, atributos, deployment e NVR playback inexistentes, mantendo-os explicitamente na matriz.

## Recomendação

C++20/MSVC x64/CMake/Ninja/vcpkg manifest; Drogon como control plane; Qt Quick+C++ como candidato preferido condicionado a licenciamento e benchmark; xc_media FFmpeg com software de referência e D3D11VA prioritário no laboratório Windows, NVDEC opcional e QSV só em hardware elegível; ONNX Runtime C++ por backend substituível, CPU oracle e avaliação WinML/CUDA/TensorRT/DirectML. SQLite preservado sem ORM pesado. Ver [arquitetura](ARQUITETURA-CPP-PROPOSTA.md) e [decisão executiva](DECISAO-ARQUITETURAL-EXECUTIVA.md).

C++ não elimina decode, IO, cópias nem dependências proprietárias. GO WITH BLOCKERS limita a próxima etapa futura ao scaffolding e provas isoladas, depois da revisão desta auditoria. Não autoriza substituir a r25. CPP-001 não foi criada.

## Esclarecimento de paridade — modo Automático e menus

MosaicTileViewModel.EffectiveStreamType e MosaicSessionViewModel.UpdateAutomaticStreamPromotion mantêm o comportamento legado: Automático promove o tile maximizado a Principal; modos Principal/Extra explícitos permanecem fixos. Não confundir com autotuning por CPU. VisualizationTabViewModel.OnTileAction tem opções que apenas exibem mensagem de integração; snapshot contextual, áudio, talk, instant-playback e gravação manual não são operações concluídas. Snapshot de evidência/IA e DAV têm caminhos próprios. Ver [UI](UI-PARIDADE-R25-PARA-CPP.md).

Revisão documental Intelbras de 23/10/2026: [addendum Intelbras/Cloud](../AUDIT-001-INTELBRAS-CLOUD-ADDENDUM.md). Preservados demais resultados.