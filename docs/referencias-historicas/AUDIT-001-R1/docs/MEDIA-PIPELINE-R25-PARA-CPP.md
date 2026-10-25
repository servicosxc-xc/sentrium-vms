# Vídeo r25 e desenho xc_media

## Fluxos atuais separados

Live: câmera/NVR → NetSDK RAW_DATA Server → cópia gerenciada única por callback → broker Device+Channel+Stream → subscriber bounded → WebSocket XCVI → Client MemoryStream/payload byte[] → fila comprimida bounded → PLAY_InputData → decoder/player nativo → HWND. OnDecoded Client só lê FrameInfo/contadores; não copia pixels para WPF. OnDraw mede callback aproximado e desenha GDI overlay. WPF hospeda HWND, não WriteableBitmap por frame.

Headless: câmera/NVR → mesmo broker comprimido quando chave compatível → subscription64 → worker FeedAsync → PlaySDK HWND nulo → callback IYUV/I420 → sampling monotônico → frame byte[] imutável compartilhado → LPR buffers e oportunidade de inferência. Motion copia plano Y amostrado a cada500 ms. PersonVehicle usa gate FIFO limitado de IDs antes de cópia/agendamento; uma inferência por vez no detector global, sem fila de frames. Stream Principal/Extra são chaves distintas; regras pertencem a uma única fonte escolhida pelo plano. Compatibilidade de dimensão/formato vem da fonte corrente; não há API de resolução por consumidor.

IA: I420 → conversão/resize nearest 416x416/pad114/BGR planar float sem /255 → ONNX YOLOX → classes/NMS → tracker → runtime de regras → evento/overlay. LPR recebe resultado do mesmo detector/tracker, aplica ROI/gate próprio e frame refs por perfil; burst→BMP→temp→Python→OpenCV/ONNX→JSON→consenso/dedup→reading/evidência/event/actions.

Snapshot de editor: último frame semântico → BMP sob demanda; snapshot de evento JPEG85 e crop93, BMP legado mantido. Thumbnails JPEG75 largura192 sob demanda com semáforo2/cache. LPR snapshot/crop vêm do frame vencedor. Não há chamada SDK de snapshot de câmera no fluxo atual.

Playback: seleção de mídia → download HTTP do DAV inteiro → cache Client → PlaySdkFilePlayer/OpenFile → HWND; pause/seek/posição por timer400ms. Isso é playback de evidência local, não busca/reprodução de timeline do DVR nem gravação VMS contínua.

Recording/evidence: AiEvidenceRecorder copia cada payload para prebuffer temporal e escreve clips DAV por evento com pre/post. Não muxa nem transcodifica. LPR SaveVideo é contrato/configuração, mas o runtime declara que não cria DAV LPR sem integração ao broker; não anunciar vídeo LPR completo. Não há recording24/7, quotas de segmentos/calendário de gravação, playback remoto ou export forense completo.

## Custos/filas/recovery

| Fronteira r25 | Ownership / limite / perda |
|---|---|
| SDK callback | buffer emprestado → byte[] (Marshal.Copy) antes de retornar |
| Broker | referências do mesmo pacote; subscriber8–2048, produtor não espera; TryWrite falho perde pacote comprimido |
| Client transporte | reconstrói payload/mensagem; fila128 default, writer aguarda espaço; TCP/WS acumulação upstream ainda possível |
| PlaySDK | buffer8 MiB Client padrão/4 MiB headless; threads Main2/Extra SDK default Client, headless1; buffers internos opacos |
| Headless I420 | sampling LPR máximo por fonte; mesma ref entre perfis/inferência quando coincide; buffers mantidos por burst |
| Motion | cópia luma reutilizada e amostragem2FPS independente |
| LPR | prebuffer por perfil até90 frames, burst2–30, trabalho32 DropOldest com auditoria; últimos drops200 |
| Evidence clips | buffer por tempo/pre até15s e pacotes ativos, sem budget rígido por bytes; cópia+Write síncrono ainda contínuos |
| Latest/pacing | classes preparadas/testadas, não ligadas ao PLAY_Play; não representam render latest-frame de produção |

Recovery: device watchdog/reconnect generation invalida sources; broker sem dados≈12s fecha subscribers; headless health5/15s + startup/inference grace; reconciliação≈5s reinicia sessão; LPR exige3frames/2s de estabilidade e frame/inferência frescos. Client policy usa backoff, lotes e30s estáveis para resetar falhas, mantém outros tiles vivos ao maximizar. Master OFF cancela geração/worker/queue LPR, libera contribuição de fonte sem desligar regras IA; ON restaura Enabled/configs intactos. Não converter essas camadas em um único timer sem preservar contratos.

## xc_media proposto

CompressedStreamHub recebe PacketLease com metadata codec/timebase/extradata/generation. Há source Intelbras e futura source RTSP/file; libavformat entra se container suportado; libavcodec mantém GOP/decode; libavutil gerencia AVFrame/hwctx; libswscale só onde CPU conversion for necessária. Recording recebe refs comprimidas por fila/IO próprios e índice de segmentos/keyframes, sem bloquear live/decode; DAV legado pode exigir PlaySDK fallback.

**RAW_DATA não é prova de Annex-B.** O script lab r25 usa `ffprobe -f dhav`; isso é indicação de experimento, não compatibilidade garantida do callback ao vivo. Fragmentação, headers, codec/extradata, PTS/DTS, áudio e discontinuities precisam de captura sanitizada/SDK oficial e teste de demux. Nenhum pacote deve ser jogado diretamente num decoder H264 só pelo nome RAW.

DecodedFrameHub compartilha FrameLease imutável por fonte/device/resolução/format/generation. Renderer usa mailbox1/latest; AI usa slot1 admitido; LPR burst mantém refs com budget. Subscribers têm lifecycle independente; último fecha recursos; mudança de formato/device lost cria nova geração e renegocia. Descarte decoded é contabilizado por consumidor; perda de compressed exige resync/drop até random-access point/restart conforme contrato, nunca queda silenciosa arbitrária. Evitar backlog com budgets de idade, bytes e inflight, não apenas contagem de mensagens.

## Decode comparado — propostas

| Caminho | Vantagem a comprovar | Custo/risco | Prioridade |
|---|---|---|---|
| Software FFmpeg | referência/compatibilidade, acesso CPU direto | CPU/threads e conversões; 16 Principal pode custar caro | obrigatório no lab como oracle |
| D3D11VA | device/texture Windows e aproximação ao renderer D3D11, vendor-neutral | formatos/support/driver; superfícies de decode nem sempre shader-readable/compartilháveis | primeiro experimento acelerado Windows |
| NVDEC via CUDA | conveniência de inferência CUDA e engine dedicada NVIDIA | interop CUDA↔D3D, driver/redistribuição e fallback; não obrigar CUDA ao produto | segundo experimento RTX3070 |
| Intel QSV | hardware Intel elegível | Ryzen5700X não tem iGPU Intel; precisa outro host | investigar depois, sem afirmar ganho neste host |

FFmpeg9.0.2 é referência candidata confirmada no upstream; fixar release/config/hash somente após build/licença/codec/suporte. Não converter a auditoria em integração de FFmpeg.

## Onde zero-copy é e não é possível

- SDK buffer emprestado → PacketLease costuma exigir cópia inicial. Ownership oficial pode permitir outra opção; não supor.
- Decode D3D11VA produz AVFrame com superfície D3D; importar ID3D11Texture2D pode evitar readback CPU. Pode exigir GPU copy/video processor/color conversion para textura Qt/render compatível. Ausência de readback não equivale a zero cópia GPU.
- NV12/P010 não são RGB: transformar via shader/compute/video processor, com stride/color range/matrix e planos corretos. Não usar path full RGB CPU antes de reduzir tile.
- `av_hwframe_transfer_data` é transferência explícita para memória acessível CPU; registrar bytes/latência. Evitar fazê-la para todos os30FPS quando AI usa6.
- IA CUDA/WinML/DirectML não aceita automaticamente qualquer textura D3D11. Interop cross-API/device/fence precisa ser provado; estágio CPU de fallback pode ser inevitável. LPR OpenCV CPU também quebra cadeia zero-copy.
- Server e Client em processos/máquinas distintas não compartilham ponteiro/superfície por simples shared_ptr. Packet transport remoto requer decode no Client; compartilhamento de decoder só dentro da mesma fronteira de processo/device ou IPC formal avaliado.

Prova futura: contador de cópias CPU/GPU e bytes por estágio, ETW/GPU profiler, frame age/present e p95 por cenário C/D, resultado de reconhecimento/golden e memória/VRAM/handles em soak. Não aprovar por FPS isolado ou CPU total sem latência e qualidade.

Referências primárias: [FFmpeg9](https://ffmpeg.org/download.html), [hardware frame API](https://ffmpeg.org/doxygen/trunk/hwcontext_8h.html), [NVDEC](https://docs.nvidia.com/video-technologies/video-codec-sdk/13.0/nvdec-video-decoder-api-prog-guide/index.html), [textura D3D11 no Qt](https://doc.qt.io/qt-6/qnativeinterface-qsgd3d11texture.html). O desenho e as prioridades são inferências/propostas desta auditoria.

## Esclarecimento de paridade — modo Automático e menus

MosaicTileViewModel.EffectiveStreamType e MosaicSessionViewModel.UpdateAutomaticStreamPromotion mantêm o comportamento legado: Automático promove o tile maximizado a Principal; modos Principal/Extra explícitos permanecem fixos. Não confundir com autotuning por CPU. VisualizationTabViewModel.OnTileAction tem opções que apenas exibem mensagem de integração; snapshot contextual, áudio, talk, instant-playback e gravação manual não são operações concluídas. Snapshot de evidência/IA e DAV têm caminhos próprios. Ver [UI](UI-PARIDADE-R25-PARA-CPP.md).