using Newtonsoft.Json.Utilities;
using Newtonsoft.Json.Converters;
using UnityEngine;

public static class AotTypeEnforcer
{
    [RuntimeInitializeOnLoadMethod]
    static void EnforceTypes()
    {
        AotHelper.EnsureType<StringEnumConverter>();
    }
}
