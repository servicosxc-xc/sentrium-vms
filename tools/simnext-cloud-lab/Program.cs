// SENTRIUM F005 laboratory-only probe for locally installed SIM Next P2P libraries.
// NOT an approved cloud provider. Do not ship or copy vendor binaries into SENTRIUM.
// No vendor secrets, endpoints or hardcoded identifiers; no code interception.
using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Security.Cryptography;

internal static class Program
{
    private static string _directory = "";
    private static string _stage = "startup";
    private static object? _instance;
    private static MethodInfo? _deletePort;
    private static MethodInfo? _cleanup;
    private static long _deviceId;
    private static ushort _localPort;

    private static int Main(string[] args)
    {
        try
        {
            if (args.Length != 4 || args[0] != "--mode" ||
                (args[1] != "inspect" && args[1] != "connect") ||
                args[2] != "--library-dir")
            {
                Console.Error.WriteLine("Uso: --mode inspect|connect --library-dir <pasta SIM Next local>");
                return 2;
            }
            _directory = Path.GetFullPath(args[3]);
            if (!Directory.Exists(_directory))
            {
                Console.Error.WriteLine("FAIL MISSING_DIRECTORY: pasta local nao encontrada.");
                return 3;
            }
            string[] components =
            {
                "SIMNext.P2PSDK.dll", "SIMNext.Core.dll",
                "SIMNext.Model.dll", "Libt2uWrapper.dll", "libt2u.dll"
            };
            foreach (var name in components)
            {
                var full = Path.Combine(_directory, name);
                Console.WriteLine($"BINARY {name}: {(File.Exists(full) ? "PRESENT" : "MISSING")}");
            }
            if (components.Any(name => !File.Exists(Path.Combine(_directory, name))))
            {
                Console.Error.WriteLine("FAIL LIBRARY_SET_INCOMPLETE: nenhum teste de conexao foi feito.");
                return 4;
            }

            // Inspect identity and architecture without calling network/native bootstrap.
            _stage = "metadata";
            foreach (string name in components)
            {
                string file = Path.Combine(_directory, name);
                var architecture = ReadPeMachine(file);
                string assembly = "NATIVE";
                try { assembly = AssemblyName.GetAssemblyName(file).Name ?? "MANAGED"; }
                catch (BadImageFormatException) { }
                Console.WriteLine($"METADATA {name}: machine={architecture}; kind={assembly}");
            }

            if (args[1] == "inspect")
            {
                Console.WriteLine("INSPECT PASS: somente metadados locais; nenhuma DLL carregada ou conexao iniciada.");
                return 0;
            }

            Console.WriteLine();
            Console.WriteLine("LAB ONLY. As bibliotecas podem ter restricoes de uso/licenca.");
            Console.WriteLine("O teste usa SOMENTE seus arquivos locais e gravador autorizado, sem copiar DLLs.");
            Console.WriteLine("Pode gerar rede Cloud real e iniciar a biblioteca proprietaria.");
            Console.Write("Digite CONFIRMO para prosseguir com o teste local: ");
            if (Console.ReadLine() != "CONFIRMO")
            {
                Console.WriteLine("CANCELLED: nenhuma conexao foi iniciada.");
                return 0;
            }
            Console.Write("Cloud ID/UUID do SEU gravador (nao sera gravado): ");
            var cloudId = Console.ReadLine()?.Trim();
            if (string.IsNullOrWhiteSpace(cloudId) || cloudId.Length > 128 ||
                cloudId.Any(char.IsWhiteSpace))
            {
                Console.Error.WriteLine("FAIL INVALID_CLOUD_ID");
                return 5;
            }
            Console.Write("Porta de servico deste gravador (1-65535, NAO assumir 37777): ");
            if (!ushort.TryParse(Console.ReadLine(), out ushort remotePort) || remotePort == 0)
            {
                Console.Error.WriteLine("FAIL INVALID_SERVICE_PORT");
                return 6;
            }

            // Private local association key, not the factory serial or Cloud UUID.
            Span<byte> id = stackalloc byte[8];
            RandomNumberGenerator.Fill(id);
            _deviceId = BitConverter.ToInt64(id) & long.MaxValue;
            if (_deviceId == 0) _deviceId = 1;

            _stage = "managed-load";
            AssemblyLoadContext.Default.Resolving += ResolveManaged;
            var wrapper = AssemblyLoadContext.Default.LoadFromAssemblyPath(
                Path.Combine(_directory, "Libt2uWrapper.dll"));
            NativeLibrary.SetDllImportResolver(wrapper, ResolveNative);
            var p2p = AssemblyLoadContext.Default.LoadFromAssemblyPath(
                Path.Combine(_directory, "SIMNext.P2PSDK.dll"));

            _stage = "api-verify";
            var type = p2p.GetTypes().SingleOrDefault(t => t.Name == "VVEyeAdapter")
                ?? throw new InvalidOperationException("Adapter type unavailable");
            var ctor = type.GetConstructor(new[] { typeof(object), typeof(object) });
            // Interface-typed constructor was observed; resolve by exactly two ref-type args.
            var constructor = ctor ?? type.GetConstructors(BindingFlags.Public | BindingFlags.Instance)
                .FirstOrDefault(c => c.GetParameters().Length == 2
                    && c.GetParameters().All(p => !p.ParameterType.IsValueType));
            var addPort = RequireMethod(type, "AddSDKPort",
                typeof(long), typeof(string), typeof(ushort), typeof(ushort));
            var isActive = RequireMethod(type, "P2pIsActive", typeof(long));
            var isRelay = RequireMethod(type, "RelayConnection", typeof(ushort));
            _deletePort = RequireMethod(type, "DeleteSDKPort", typeof(long), typeof(ushort));
            _cleanup = RequireMethod(type, "Cleanup");
            if (constructor == null)
                throw new InvalidOperationException("Public adapter constructor unavailable");
            _instance = constructor.Invoke(new object?[] { null, null });
            Console.WriteLine("ADAPTER_READY: interface local carregada; autenticacao Cloud ainda nao comprovada.");

            _stage = "add-port";
            var raw = addPort.Invoke(_instance, new object[] { _deviceId, cloudId, remotePort, (ushort)0 });
            int result = Convert.ToInt32(raw);
            if (result <= 0 || result > ushort.MaxValue)
            {
                Console.WriteLine("TUNNEL NOT_READY: AddSDKPort nao retornou porta valida.");
                return 7;
            }
            _localPort = (ushort)result;
            Console.WriteLine("TUNNEL PORT_RETURNED: uma porta local foi retornada.");

            _stage = "state-check";
            bool active = Convert.ToBoolean(isActive.Invoke(_instance, new object[] { _deviceId }));
            bool relay = Convert.ToBoolean(isRelay.Invoke(_instance, new object[] { _localPort }));
            Console.WriteLine("TUNNEL STATE: " + (active ? "ACTIVE" : "UNKNOWN_OR_INACTIVE"));
            Console.WriteLine("TUNNEL ROUTE: " + (!active ? "UNKNOWN" : relay ? "RELAY_REPORTED" : "NOT_RELAY_REPORTED"));
            if (!active)
                return 8;

            var listeners = IPGlobalProperties.GetIPGlobalProperties()
                .GetActiveTcpListeners().Where(ep => ep.Port == _localPort).ToArray();
            bool loopbackOnly = listeners.Length > 0 &&
                listeners.All(ep => IPAddress.IsLoopback(ep.Address));
            Console.WriteLine("LOCAL TCP LISTENER: " + (listeners.Length > 0 ? "OBSERVED" : "NOT_OBSERVED"));
            Console.WriteLine("BIND SCOPE: " + (loopbackOnly ? "LOOPBACK_ONLY" : "NOT_PROVEN_LOOPBACK"));
            if (!loopbackOnly)
            {
                Console.WriteLine("SAFETY STOP: nao usar o endpoint no NetSDK se houver exposicao fora do loopback.");
                return 9;
            }
            Console.WriteLine("TUNNEL LAB READY: 127.0.0.1:" + _localPort);
            Console.WriteLine("NAO comprova login NetSDK, streaming ou permissao de redistribuicao.");
            Console.WriteLine("Mantenha esta janela aberta para a proxima etapa; ENTER encerra e libera o tunel.");
            Console.ReadLine();
            return 0;
        }
        catch (Exception)
        {
            Console.Error.WriteLine("LAB FAIL stage=" + _stage +
                ": erro de compatibilidade/execucao; detalhes potencialmente sensiveis ocultados.");
            return 10;
        }
        finally
        {
            // Release only while the vendor instance is alive; protect against native crashes.
            if (_instance != null)
            {
                try
                {
                    if (_localPort != 0)
                        _deletePort?.Invoke(_instance, new object[] { _deviceId, _localPort });
                }
                catch { Console.Error.WriteLine("CLEANUP WARNING: DeleteSDKPort falhou."); }
                try { _cleanup?.Invoke(_instance, Array.Empty<object>()); }
                catch { Console.Error.WriteLine("CLEANUP WARNING: Cleanup falhou."); }
                if (_instance is IDisposable d)
                    try { d.Dispose(); } catch { }
            }
        }
    }

    private static MethodInfo RequireMethod(Type type, string name, params Type[] parameterTypes)
    {
        return type.GetMethod(name, BindingFlags.Public | BindingFlags.Instance,
                binder: null, types: parameterTypes, modifiers: null)
            ?? throw new MissingMethodException(type.FullName, name);
    }

    private static Assembly? ResolveManaged(AssemblyLoadContext context, AssemblyName name)
    {
        if (string.IsNullOrWhiteSpace(name.Name) || name.Name.IndexOfAny(
            Path.GetInvalidFileNameChars()) >= 0)
            return null;
        string candidate = Path.Combine(_directory, name.Name + ".dll");
        return File.Exists(candidate) ? context.LoadFromAssemblyPath(candidate) : null;
    }

    private static IntPtr ResolveNative(string name, Assembly _, DllImportSearchPath? __)
    {
        string filename = Path.GetFileNameWithoutExtension(name);
        if (filename.Equals("libt2u", StringComparison.OrdinalIgnoreCase))
            return NativeLibrary.Load(Path.Combine(_directory, "libt2u.dll"));
        return IntPtr.Zero;
    }

    private static string ReadPeMachine(string path)
    {
        using var f = File.OpenRead(path);
        if (f.Length < 0x40) return "INVALID";
        Span<byte> header = stackalloc byte[64];
        f.ReadExactly(header);
        if (header[0] != (byte)'M' || header[1] != (byte)'Z') return "INVALID";
        int peOffset = BitConverter.ToInt32(header[0x3c..0x40]);
        if (peOffset < 0 || (long)peOffset + 6 > f.Length) return "INVALID";
        f.Position = peOffset;
        Span<byte> pe = stackalloc byte[6];
        f.ReadExactly(pe);
        if (pe[0] != (byte)'P' || pe[1] != (byte)'E' || pe[2] != 0 || pe[3] != 0)
            return "INVALID";
        ushort machine = BitConverter.ToUInt16(pe[4..6]);
        return machine switch
        {
            0x8664 => "AMD64",
            0x014c => "I386",
            0xAA64 => "ARM64",
            _ => "OTHER"
        };
    }
}
