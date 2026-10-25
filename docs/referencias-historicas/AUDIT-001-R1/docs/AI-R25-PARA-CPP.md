# IA r25 para C++

## Oracle atual

YoloXObjectDetector.cs usa models/yolox_tiny.onnx (20.219.662 bytes, confirmado pelo manifesto) com SHA256 **427CC366D34E27FF7A03E2899B5E3671425C262EA2291F88BB942BC1CC70B0F7**. O nome sozinho não garante mesma variante; preservar hash/export/opset/input/output antes de qualquer otimização. Não foi executada inferência nem carregado o modelo nesta auditoria.

I420: Y e planos U/V separados com dimensões/chromaWidth=width/2; converter YUV limitado via aritmética inteira atual (298c/409e/100d/208e/516d), clamp0–255. Entrada 1x3x416x416, **BGR float0–255, sem /255**, nearest-neighbor e letterbox114 ancorado no canto superior esquerdo, não letterbox central YOLOv9 do LPR. Ratio=min(416/h,416/w). Span planar/tensor reutilizado sob lock na r25; manter equivalência antes de SIMD/GPU. Resultados ONNX: rows3549/attrs85, grids strides8/16/32, objectness*classe; manter somente IDs0,1,2,3,5,7 (person/bicycle/car/motorcycle/bus/truck), class mapping e NMS por Kind. NMS agrupa todas as classes Vehicle pelo mesmo Kind, não por classId: é comportamento existente que deve entrar no golden.

Options: ORT sequential, memory pattern false, graph optimizations; tentar DML e fallback CPU. RuntimeDescription e erro real devem permanecer observáveis, sem registrar falha de GPU como sucesso. Threshold do detector semântico considera global, regras ativas e LPR para não cortar antes do perfil; detector LPR/OCR globalmente validados permanecem35%/65% por padrão, e seus fallbacks próprios não devem ser confundidos com threshold COCO/worker permissivo.

MotionDetector: luma difference modelo-free, stride8, delta28, proporção0,07,3quiet samples para reset. É telemetria/contexto em PersonVehicle; não duplicar evento motion nesse modo. Tracker não é ByteTrack/DeepSORT: matching greedy por Kind, score0,72IoU+0,28proximidade, maxMisses5/minHits2/minIoU0,08/distância1,8 default; pré-filtra classes, trackIDs e hits/misses. Reset limpa tracks, não é reset universal de IDs/histórico. Não trocar algoritmo na migração inicial.

AiRuleRuntime: pending config aplicada na inferência, update idempotente não reinicia tracker, ocorrência por regra/objeto, ausência padrão5s, reassociação curta exclusiva, cooldown legado optativo, ROI por ponto inferior central e linha2–12pontos/direções/AtoB/BtoA/limite de segmento. Eventos evitam duplicação por objeto estacionário e por reentrada espúria. Mudança numa regra não reseta as outras. Agenda semanal/schedule profissional não está nos contratos atuais: é backlog, não funcionalidade pronta.

## Camada nativa e backends

IInferenceBackend/model context é fronteira de sessão/submit/diagnostics/cancel, não chamada virtual por pixel. Tipos de tensor/FrameLease do xc_core e preprocess explícito no xc_ai; política de pools/contextos fica fora do modelo. CPU ORT C++ como referência inicial; não obrigar toda a inferência a passar pela API de alto nível da UI.

| Backend | Proposta / gate |
|---|---|
| ORT C++ CPU | oracle e fallback obrigatório; threads/arena/serialização medidos |
| ORT CUDA | comparar na RTX, transfer/pre/post e versões cuDNN/CUDA/ORT compatíveis; não é obrigatório |
| ORT TensorRT | etapa posterior; builds/caches/shape/precisão/model coverage e paridade; nenhuma promessa de FP16/INT8 sem avaliação |
| ORT DirectML | compatibilidade r25; está em **sustained engineering**, suportado mas evolução orientada a WinML; não arquitetura final automática |
| WinML moderno | avaliar para novo Windows, deployment/EP/device policy e versão mínima real; não confundir com API in-box legada Windows.AI.MachineLearning |

A observação de manutenção DirectML e recomendação de avaliar WinML é suportada pela [documentação ORT](https://onnxruntime.ai/docs/execution-providers/DirectML-ExecutionProvider.html). [WinML moderno](https://learn.microsoft.com/en-us/windows/ai/new-windows-ml/overview) usa ONNX Runtime e providers geridos pelo Windows; confirmar suporte do Windows alvo/package/driver e termos antes de escolher. [CUDA](https://onnxruntime.ai/docs/execution-providers/CUDA-ExecutionProvider.html) e [TensorRT](https://onnxruntime.ai/docs/execution-providers/TensorRT-ExecutionProvider.html) têm matrizes de compatibilidade próprias.

## Paridade e qualidade

Golden de frame→tensor integral, frame→detections com tolerância definida, detections→tracker→ROI/linha/evento e timestamps determinísticos. Cuidado com rounding .NET vs C++, layout/stride, BGR/RGB, range de cor, tie-order no sorting/NMS, clocks e replay com frames fora de ordem. Comparar matched detections/geometria/confiança e decisões de evento, não apenas contagem. Testes r25 de preprocess cobrem três orientações e padding; os 151 não provam equivalência GPU ou dataset real completo.

Medir preprocess/Run/postprocess separadamente, admitted/inference FPS, inflight/queue/frame age/drops, consumo e engine GPU por provider, precisão/cobertura de modelo. Proteger concorrência de tensor/contexto. Sem reduzir thresholds ou alterar modelo para obter benchmark melhor.