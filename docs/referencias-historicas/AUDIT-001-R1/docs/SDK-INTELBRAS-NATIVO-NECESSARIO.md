# Intelbras/Dahua nativo necessário

## Proveniência e alcance real

A r25 informa no README legado o pacote `General_NetSDK_ChnEng_CSharp64_IS_V3.050.0000005.3.R.190304.zip`. Isso identifica uma origem declarada, não prova suporte atual, licença ou cadeia de custódia dos 15 binários. Foram lidos headers PE/import/export sem carregar DLLs. [Artefatos PE/hashes](../artifacts/SDK-NATIVE-PE.json), [P/Invoke completo](../artifacts/SDK-PINVOKE-R25.json), [callsites](../artifacts/SDK-CALLSITES-R25.json), [reachability](../artifacts/SDK-REACHABILITY.json).

Foram encontrados 151 imports lexicais LIBRARYNETSDK, dois LIBRARYCONFIGSDK e 60 de dhplay somando produção/labs; há outros P/Invoke Windows. Os 242 registros totais incluem repetição, não equivalem a 242 funções usadas. O wrapper contém 36.043 linhas entre NetSDK.cs/NetSDKStruct.cs/OriginalSDK.cs; não portar esse volume manualmente.

**Somente dez métodos NETClient são chamados por código XC fora do wrapper:**

| Método XC/C# | Export nativo alcançado |
|---|---|
| NETClient.Cleanup | CLIENT_Cleanup |
| NETClient.GetLastError | CLIENT_GetLastError |
| NETClient.Init | CLIENT_InitEx |
| NETClient.Login | CLIENT_LoginEx2 |
| NETClient.Logout | CLIENT_Logout |
| NETClient.PTZControl | CLIENT_DHPTZControlEx2 |
| NETClient.RealPlay | CLIENT_RealPlayEx |
| NETClient.SetAutoReconnect | CLIENT_SetAutoReconnect |
| NETClient.SetRealDataCallBack | CLIENT_SetRealDataCallBackEx2 |
| NETClient.StopRealPlay | CLIENT_StopRealPlayEx |

O arquivo de reachability mostra o método/linha no wrapper. Login usado é CLIENT_LoginEx2; login high-level-security, playback remoto e snapshots SDK não devem ser declarados ativos só porque seus imports existem. Inicialização/disconnect/autoreconnect são callbacks retidos por delegates no C#. O contrato C++ precisa preservar sua vida até stop/cleanup completo.

## PlaySDK efetivamente declarado em produção

`PLAY_CloseFile, PLAY_CloseStream, PLAY_GetBufferValue, PLAY_GetFileTime, PLAY_GetFreePort, PLAY_GetLastError, PLAY_GetPlayedTime, PLAY_GetSourceBufferRemain, PLAY_InputData, PLAY_OpenFile, PLAY_OpenStream, PLAY_Pause, PLAY_Play, PLAY_ReleasePort, PLAY_RigisterDrawFun, PLAY_SetDecCBStream, PLAY_SetDecCallBackEx, PLAY_SetDecodeThreadNum, PLAY_SetPlayPos, PLAY_SetStreamOpenMode, PLAY_Stop`.

Entry point histórico é **PLAY_RigisterDrawFun** (grafia do export), embora método gerenciado seja RegisterDrawFun. Não corrigir nomes presumidos. Client usa OpenFile/Pause/SetPlayPos/GetFileTime/GetPlayedTime para DAV local, além de stream/headless. Conferir cada export contra a versão oficial escolhida antes de linkar; declaração sozinha não prova caminho executado em todas as configurações.

## DLLs x64 presentes

| DLL | Exports nomeados | Imports PE observados |
|---|---:|---|
| avnetsdk.dll | 136 | KERNEL32.dll, WS2_32.dll, Infra.dll, Stream.dll, NetFramework.dll, StreamSvr.dll, WINMM.dll, ADVAPI32.dll |
| dhconfigsdk.dll | 5 | KERNEL32.dll |
| dhnetsdk.dll | 1015 | KERNEL32.dll, WS2_32.dll, WINMM.dll, ADVAPI32.dll |
| dhplay.dll | 275 | KERNEL32.dll, USER32.dll, GDI32.dll, ole32.dll, WINMM.dll, DSOUND.dll, DDRAW.dll |
| fisheye.dll | 13 | ole32.dll, OLEAUT32.dll, KERNEL32.dll |
| ImageAlg.dll | 7 | MSVCR80.dll, KERNEL32.dll |
| Infra.dll | 427 | KERNEL32.dll |
| IvsDrawer.dll | 33 | KERNEL32.dll, USER32.dll, GDI32.dll, MSVCP80.dll, MSVCR80.dll |
| Json.dll | 211 | KERNEL32.dll |
| libcurl.dll | 61 | WLDAP32.dll, WS2_32.dll, LIBEAY32.dll, SSLEAY32.dll, KERNEL32.dll, MSVCR80.dll |
| libeay32.dll | 3683 | WS2_32.dll, GDI32.dll, ADVAPI32.dll, USER32.dll, KERNEL32.dll |
| NetFramework.dll | 448 | Infra.dll, WINMM.dll, KERNEL32.dll, WS2_32.dll |
| ssleay32.dll | 321 | LIBEAY32.dll, KERNEL32.dll |
| Stream.dll | 37 | json.dll, Infra.dll, KERNEL32.dll |
| StreamSvr.dll | 248 | Infra.dll, json.dll, NetFramework.dll, Stream.dll, KERNEL32.dll, USER32.dll, ADVAPI32.dll, WS2_32.dll |

Imports PE são dependências diretas estáticas, não toda árvore dinâmica/DelayLoad/plugins instalados. libeay32/ssleay32 indicam necessidade de apurar versão/suporte do pacote antigo; o nome não basta para atribuir CVE. A v2 deve usar kit coeso do fornecedor, não atualizar DLLs individuais arbitrariamente.

## Comportamento e limites da r25

- DirectTCP: runtime serializado em thread dedicada, BlockingCollection256, init/login/logout/realplay/callback/stop e cleanup com estados Created/Initializing/Running/Stopping/Stopped/StoppedWithoutCleanup.
- PTZ preset chama CLIENT_DHPTZControlEx2 fora do runtime central (IntelbrasDeviceService:720): encapsular sob a mesma política contratual, mantendo câmera alvo independente.
- Channels: contagem derivada de login/ExpectedChannels; DTOs enumeram índices zero-based e nomes gerados `Canal NN`. Não há inventário completo de capabilities nem consulta universal de nomes reais do equipamento. Estado DeviceDto não prova suporte de codec/PTZ/snapshot/recording.
- Media: RealPlay headless HWND nulo, RAW_DATA callbacks; source key Device+Channel+Stream e ConnectionGeneration. Request Extra não faz fallback silencioso no broker; probe/runtime tem opção separada allowMainFallback. Preservar distinção.
- Disconnect/autoreconnect invalidam gerações/fontes; callback copia pacote uma vez e publica refs. Stop último subscriber fecha RealPlay; reader completion provoca reassinatura. Watchdog e retry continuam necessários.
- Snapshot do produto vem do frame decodificado e encoder, não de SnapPictureEx. Evidence DAV é concatenação de pacotes guardados; reprodução DVR por busca/calendário/download não está implementada.

## Atualização Intelbras — 23/10/2026

Ver o [addendum Intelbras/Cloud](../AUDIT-001-INTELBRAS-CLOUD-ADDENDUM.md): evidências posteriores fornecidas pelo usuário, não probes repetidos nesta revisão. NetSDK/PlaySDK nativos C++ x64 disponíveis em `General_NetSDK_3.050_PlaySDK_3.042`: dhnetsdk.h, dhnetsdk.lib, dhconfigsdk.lib, dhplay.h, dhplay.lib, DLLs e exemplos nativos. Ausência de SDK C++ deixa de ser blocker. O fato histórico de esses headers/libs não estarem na árvore r25 não significa indisponibilidade do kit externo.

Cloud: SIMNext.P2PSDK.dll → Libt2uWrapper.dll → libt2u.dll; wrapper gerenciado não é obrigação arquitetural. Bootstrap observado t2u_init(server,1250,key), UDP/1250; endereço e credential somente oficiais. AddSDKPort usa t2u_add_port_v3 e polling de port_status. Túnel por dispositivo/sessão Cloud é compartilhado por login, controle e RealPlay; não destruí-lo por tile. Relay = proxy==1; Direct = connected && proxy!=1; dictionary não prova conectividade. ABI provável e exports estão no addendum, sem tratá-los como contrato definitivo.

## Checklist atualizado

| Item | Estado / pendência real |
|---|---|
| Headers/import libs NetSDK/PlaySDK x64 e exemplos | localizados no pacote informado; não reconstruir; validar ABI/versões/hashes do kit e subset usado |
| DLLs/CRT/dependências | kit antigo; verificar compatibilidade/paridade com equipamentos e SIM Next atual; não trocar DLLs isoladamente |
| libt2u e exports | identificados; validar assinaturas/ABI/packing/callbacks em probe futuro autorizado |
| EULA/redistribuição | autorização explícita libt2u; confirmar termos demais binários do kit |
| Bootstrap | servidor/configuração/credential oficial; não extrair segredo privado do SIM Next |
| Standalone | confirmação Intelbras e Windows limpo sem SIM Next |
| Runtime | reconexão, P2P↔Relay, lifetime, shutdown e segurança do listener em campo |
| Mídia/capabilities | preservar testes RAW/DHAV, canais/PTZ, timestamps e codec do relatório original |

IB-01 a IB-06 bloqueiam declarar integração pronta para produção, não Foundation desacoplada. Não foi carregada DLL nem implementado adaptador nesta revisão.