# LPR r25 para C++ — comportamento antes da migração

## Fontes lidas

ILprEngine/FastAlprBridgeEngine/PlateDeduplicator/LprPlateNormalizer/LprTemporalConsensus, Server/Services/Lpr/* e scripts/lpr/fastalpr_worker.py. Também foram lidos estaticamente fontes/metadados instalados do venv preferencial, sem importá-los/executá-los: fast-alpr0.4.0, fast-plate-ocr1.1.0 e open-image-models0.6.0. [Proveniência/hashes](../artifacts/DEPENDENCIAS-PYTHON-OBSERVADAS.json). Isso descreve a instalação observada, não garante que todo ambiente r25 resolva as mesmas dependências: requirements fixa FastALPR0.4/ORT1.24.4 mas transitivas têm faixas; constraints-lock não está na fonte distribuída.

## Pipeline e estado que precisam sobreviver

Fonte/decode/detector/tracker compartilhados por câmera+stream → ProcessFrame detections → estado por profile → ponto inferior central do veículo → ROI/gate/direção plausível → burst com pre/post → ranking de qualidade → frame candidato → ILprEngine → normalização BR → consenso de frames físicos → dedup por perfil → persist reading/attempt antes de publish/actions → snapshot/crop/dataset/ground truth e Muralha async.

Master OFF persiste flag, bloqueia admissão, cancela geração, drena/descarta fila, encerra worker e limpa estados; não muda Enabled/ROI/gate/threshold/actions/históricos. ON restaura os mesmos perfis habilitados. Regras IA que compartilham fonte continuam. PipelineHealth mantém Running/Degraded/Recovering/Stalled/Disabled, frame/inference ages, stalls/recoveries/duração e histerese3frames/2s com janelas5/15s; não aceitar um frame solitário como recovery.

Por perfil: gate de dois pontos, ROI poligonal, direções Both/AtoB/BtoA; cruzamento pode usar um lado dentro da ROI e exige deslocamento≤0,35 normalizado. Prebuffer padrão350ms/post900ms/FPS6/MaxBurst8, clamps conforme store/runtime e máximo90 amostras buffer. Frames imutáveis compartilhados entre perfis; detections/bursts e state independentes. Fila de trabalho32 DropOldest com motivo, profile, idade, high-water, in-flight e reset não destrutivo.

Qualidade gerenciada calcula brilho/clipping/gradientes na região do veículo, score0,48sharp+0,37exposure+0,15clipping, day/night/mixed; ordena melhores primeiro mantendo frames úteis. Worker também calcula métricas próprias OpenCV (Laplacian variance); não substituir uma definição pela outra. Preservar attempts com candidatos/variantes/fallback/resultados e dataset rejeitado separado do manual.

## Engine/worker atual em detalhe

Bridge exige BMP, grava arquivo temporário e envia linha JSON `{id,image,region}` ao processo Python persistente. Handshake ready/error; timeouts120s inicialização/20s reconhecimento, backoff base2s até30s, correlação de request id; timeout/protocolo inválido invalida geração e próxima chamada reinicia worker. stderr drenado, temp removido em finally, cancel/Master encerra processo. A v2 remove esse transporte do runtime principal, mas preserva estados/diagnósticos/falhas de persistência.

Worker escolhe DML quando disponível (ou CPU por XC_LPR_PROVIDER), instancia detector **yolo-v9-t-384-license-plate-end2end**, OCR **cct-xs-v2-global-model**, detector permissivo0,20 antes dos filtros do perfil. Crop da região do veículo usa coordenadas normalizadas, round/clamps e offsets de volta ao frame completo. `DefaultDetector` observado usa open-image-models0.6 YOLOv9: letterbox central INTER_LINEAR/pad114, RGB NCHW float/255; saída end-to-end com colunas batch/box/class/score, desfaz padding/scale e converte boxes a int. Não adicionar NMS duplicado sem confirmar export end-to-end.

OCR DefaultOCR converte crop BGR para grayscale ou RGB conforme YAML. fast-plate-ocr redimensiona pela configuração (dimensões/interpolation/keep_aspect/pad); entrada **NHWC uint8**, normalização está dentro do modelo observado. Pós-processamento reshapa para slots×alphabet, argmax por posição, remove padding apenas no fim, confiança por caractere; ALPR/worker usam média. **Não é decoder CTC genérico**, nem é correto normalizar /255 fora do modelo por convenção. Region head opcional depende de outputs/config, não inventar suporte caso o artefato não forneça.

O YAML exato de cct_xs_v2_global, alfabeto, pad, maxslots, dimensões e labels de região não está na árvore r25; o código o baixa de release/cache externa. Não foi lido cache/modelo operacional nem baixado peso nesta auditoria. URL de YAML upstream não foi recuperável pelo browser da auditoria; não preencher valores por adivinhação. Esses artefatos/hash/modelcard/licença são gate P0 da CPP-LPR. O nome do modelo não basta.

## Variantes exatas observadas

Original sempre. OCR original forte significa texto compatível BR e confiança≥0,78. Se não forte, adicionar CLAHE no canal L em Lab (clipLimit2,2/grid8×8), depois uma variante: noite se média<105 ou darkRatio>0,38 → gamma0,68 sobre CLAHE; highlight se média>175 ou brightRatio>0,18 → gamma1,45; senão unsharp (Gaussian sigma1,15 e pesos1,55/-0,55). LUT gamma truncada uint8. Ordenar candidatos por detectorConfidence×max(0,01,OCR), limitar12 por frame. Variantes do mesmo timestamp não contam como votos independentes.

Filtro normal do Server por perfil: detector0,35/OCR0,65 defaults. Fallback: detector≥max(0,20,threshold−0,10), OCR≥max(0,85,thresholdOCR), padrão BR válido, quality≥0,30 e contexto veículo/ROI/gate. Consenso janela2,5s, mínimo2 frames físicos distintos; candidato equivalente no mesmo frame conserva melhor produto de confiança. Normalização ASCII uppercase/remoção separadores, padrões legado/Mercosul e conversões posicionais; empate escolhe menor número de substituições (legado em empate). Dedup chave profile+canonical plate, renovação enquanto observado é opt-in e default false. Nova passagem/EventId da mesma placa pode ser evento distinto.

## Desenho xc_lpr e gates

ModelRuntime ORT C++ → detector placa → crop → OCR por slots/config → candidatos imutáveis → políticas de normalização/consenso/dedup puras. Interfaces de contexto/modelo fora do loop pixel, pre/post/IO separados e buffers de pool. OpenCV C++ core/imgproc/imgcodecs é candidato para equivalência das variantes/crop/interpolação; não usar Python ou Tesseract substituto silencioso. GPU preprocess é segunda etapa, depois de golden CPU.

1. Obter detector.onnx + OCR.onnx + YAML/config + labels + licença/modelcard/proveniência/hashes e matrizes IO/opset, sem modificar os ativos r25.
2. Congelar ambiente oracle e fixtures autorizadas de dia/noite/reflexo/ambíguas/múltiplos perfis. Comparar candidatos por frame, crop offsets, tensor, confidence, variantes, consenso/dedup e reading/evento, não só texto final.
3. Validar variantes/rounding/Unicode/qualidade e regra same-frame. Preservar thresholds/modelo e metadata dos eventos.
4. Executar failure/cancel/Master/queue/recovery/persist-before-publish, ground truth/reviews/dataset e proteção de evidência.
5. Medir latência por detector/OCR/burst/consenso, false-positive/false-negative e CPU/VRAM. Só depois retirar bridge do caminho principal. Runtime final C++ é objetivo; não foi implementado nesta auditoria.

Licenças dos pacotes upstream são MIT, mas isso **não prova automaticamente licença dos pesos/dados**. YOLOX Apache2 source também não resolve licença do artefato exportado. Bloqueio comercial e de paridade permanece até o dossiê de assets. Ver [licenças](DEPENDENCIAS-E-LICENCAS.md).