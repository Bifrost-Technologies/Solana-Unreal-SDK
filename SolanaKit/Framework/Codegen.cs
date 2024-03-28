/*
 *  Unreal Engine .NET 6 integration 
 *  Copyright (c) 2021 Stanislav Denisov
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

using System;
using System.IO;
using System.Collections.Generic;
using System.Numerics;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace UnrealEngine.Framework {
	// Automatically generated

	internal static class Shared {
		internal const int checksum = 0x2F0;
		internal static Dictionary<int, IntPtr> userFunctions = new();
		private const string dynamicTypesAssemblyName = "UnrealEngine.DynamicTypes";
		private static readonly ModuleBuilder moduleBuilder = AssemblyBuilder.DefineDynamicAssembly(new(dynamicTypesAssemblyName), AssemblyBuilderAccess.RunAndCollect).DefineDynamicModule(dynamicTypesAssemblyName);
		private static readonly Type[] delegateCtorSignature = { typeof(object), typeof(IntPtr) };
		private static Dictionary<string, Delegate> delegatesCache = new();
		private static Dictionary<string, Type> delegateTypesCache = new();
		private const MethodAttributes ctorAttributes = MethodAttributes.RTSpecialName | MethodAttributes.HideBySig | MethodAttributes.Public;
		private const MethodImplAttributes implAttributes = MethodImplAttributes.Runtime | MethodImplAttributes.Managed;
		private const MethodAttributes invokeAttributes = MethodAttributes.Public | MethodAttributes.HideBySig | MethodAttributes.NewSlot | MethodAttributes.Virtual;
		private const TypeAttributes delegateTypeAttributes = TypeAttributes.Class | TypeAttributes.Public | TypeAttributes.Sealed | TypeAttributes.AnsiClass | TypeAttributes.AutoClass;

		internal static unsafe Dictionary<int, IntPtr> Load(IntPtr* events, IntPtr functions, Assembly pluginAssembly) {
			int position = 0;
			IntPtr* buffer = (IntPtr*)functions;

			unchecked {
				int head = 0;
				IntPtr* assertFunctions = (IntPtr*)buffer[position++];

				Assert.outputMessage = (delegate* unmanaged[Cdecl]<byte[], void>)assertFunctions[head++];
			}

			unchecked {
				int head = 0;
				IntPtr* debugFunctions = (IntPtr*)buffer[position++];

				Debug.log = (delegate* unmanaged[Cdecl]<LogLevel, byte[], void>)debugFunctions[head++];
				Debug.exception = (delegate* unmanaged[Cdecl]<byte[], void>)debugFunctions[head++];
				Debug.addOnScreenMessage = (delegate* unmanaged[Cdecl]<int, float, int, byte[], void>)debugFunctions[head++];
				Debug.clearOnScreenMessages = (delegate* unmanaged[Cdecl]<void>)debugFunctions[head++];
				Debug.drawBox = (delegate* unmanaged[Cdecl]<in Vector3, in Vector3, in Quaternion, int, Bool, float, byte, float, void>)debugFunctions[head++];
				Debug.drawCapsule = (delegate* unmanaged[Cdecl]<in Vector3, float, float, in Quaternion, int, Bool, float, byte, float, void>)debugFunctions[head++];
				Debug.drawCone = (delegate* unmanaged[Cdecl]<in Vector3, in Vector3, float, float, float, int, int, Bool, float, byte, float, void>)debugFunctions[head++];
				Debug.drawCylinder = (delegate* unmanaged[Cdecl]<in Vector3, in Vector3, float, int, int, Bool, float, byte, float, void>)debugFunctions[head++];
				Debug.drawSphere = (delegate* unmanaged[Cdecl]<in Vector3, float, int, int, Bool, float, byte, float, void>)debugFunctions[head++];
				Debug.drawLine = (delegate* unmanaged[Cdecl]<in Vector3, in Vector3, int, Bool, float, byte, float, void>)debugFunctions[head++];
				Debug.drawPoint = (delegate* unmanaged[Cdecl]<in Vector3, float, int, Bool, float, byte, void>)debugFunctions[head++];
				Debug.flushPersistentLines = (delegate* unmanaged[Cdecl]<void>)debugFunctions[head++];
			}

			unchecked {
				int head = 0;
				IntPtr* objectFunctions = (IntPtr*)buffer[position++];

				Object.getName = (delegate* unmanaged[Cdecl]<IntPtr, byte[], void>)objectFunctions[head++];
				Object.getBool = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref bool, Bool>)objectFunctions[head++];
				Object.getByte = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref byte, Bool>)objectFunctions[head++];
				Object.getShort = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref short, Bool>)objectFunctions[head++];
				Object.getInt = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref int, Bool>)objectFunctions[head++];
				Object.getLong = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref long, Bool>)objectFunctions[head++];
				Object.getUShort = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref ushort, Bool>)objectFunctions[head++];
				Object.getUInt = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref uint, Bool>)objectFunctions[head++];
				Object.getULong = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref ulong, Bool>)objectFunctions[head++];
				Object.getFloat = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref float, Bool>)objectFunctions[head++];
				Object.getDouble = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref double, Bool>)objectFunctions[head++];
				Object.getEnum = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ref int, Bool>)objectFunctions[head++];
				Object.getString = (delegate* unmanaged[Cdecl]<IntPtr, byte[], byte[], Bool>)objectFunctions[head++];
				Object.getText = (delegate* unmanaged[Cdecl]<IntPtr, byte[], byte[], Bool>)objectFunctions[head++];
				Object.setBool = (delegate* unmanaged[Cdecl]<IntPtr, byte[], Bool, Bool>)objectFunctions[head++];
				Object.setByte = (delegate* unmanaged[Cdecl]<IntPtr, byte[], byte, Bool>)objectFunctions[head++];
				Object.setShort = (delegate* unmanaged[Cdecl]<IntPtr, byte[], short, Bool>)objectFunctions[head++];
				Object.setInt = (delegate* unmanaged[Cdecl]<IntPtr, byte[], int, Bool>)objectFunctions[head++];
				Object.setLong = (delegate* unmanaged[Cdecl]<IntPtr, byte[], long, Bool>)objectFunctions[head++];
				Object.setUShort = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ushort, Bool>)objectFunctions[head++];
				Object.setUInt = (delegate* unmanaged[Cdecl]<IntPtr, byte[], uint, Bool>)objectFunctions[head++];
				Object.setULong = (delegate* unmanaged[Cdecl]<IntPtr, byte[], ulong, Bool>)objectFunctions[head++];
				Object.setFloat = (delegate* unmanaged[Cdecl]<IntPtr, byte[], float, Bool>)objectFunctions[head++];
				Object.setDouble = (delegate* unmanaged[Cdecl]<IntPtr, byte[], double, Bool>)objectFunctions[head++];
				Object.setEnum = (delegate* unmanaged[Cdecl]<IntPtr, byte[], int, Bool>)objectFunctions[head++];
				Object.setString = (delegate* unmanaged[Cdecl]<IntPtr, byte[], byte[], Bool>)objectFunctions[head++];
				Object.setText = (delegate* unmanaged[Cdecl]<IntPtr, byte[], byte[], Bool>)objectFunctions[head++];
			}

			unchecked {
				Type[] types = pluginAssembly.GetTypes();

				foreach (Type type in types) {
					MethodInfo[] methods = type.GetMethods();

					if (type.Name == "Main" && type.IsPublic) {
						foreach (MethodInfo method in methods) {
							if (method.IsPublic && method.IsStatic && !method.IsGenericMethod) {
								ParameterInfo[] parameterInfos = method.GetParameters();

								if (parameterInfos.Length <= 1) {
									if (method.Name == "OnWorldBegin") {
										if (parameterInfos.Length == 0)
											events[0] = GetFunctionPointer(method);
										else
											throw new ArgumentException(method.Name + " should not have arguments");

										continue;
									}

									if (method.Name == "OnWorldPostBegin") {
										if (parameterInfos.Length == 0)
											events[1] = GetFunctionPointer(method);
										else
											throw new ArgumentException(method.Name + " should not have arguments");

										continue;
									}

									if (method.Name == "OnWorldPrePhysicsTick") {
										if (parameterInfos.Length == 1 && parameterInfos[0].ParameterType == typeof(float))
											events[2] = GetFunctionPointer(method);
										else
											throw new ArgumentException(method.Name + " should have a float argument");

										continue;
									}

									if (method.Name == "OnWorldDuringPhysicsTick") {
										if (parameterInfos.Length == 1 && parameterInfos[0].ParameterType == typeof(float))
											events[3] = GetFunctionPointer(method);
										else
											throw new ArgumentException(method.Name + " should have a float argument");

										continue;
									}

									if (method.Name == "OnWorldPostPhysicsTick") {
										if (parameterInfos.Length == 1 && parameterInfos[0].ParameterType == typeof(float))
											events[4] = GetFunctionPointer(method);
										else
											throw new ArgumentException(method.Name + " should have a float argument");

										continue;
									}

									if (method.Name == "OnWorldPostUpdateTick") {
										if (parameterInfos.Length == 1 && parameterInfos[0].ParameterType == typeof(float))
											events[5] = GetFunctionPointer(method);
										else
											throw new ArgumentException(method.Name + " should have a float argument");

										continue;
									}

									if (method.Name == "OnWorldEnd") {
										if (parameterInfos.Length == 0)
											events[6] = GetFunctionPointer(method);
										else
											throw new ArgumentException(method.Name + " should not have arguments");

										continue;
									}
								}
							}
						}
					}

					foreach (MethodInfo method in methods) {
						if (method.IsPublic && method.IsStatic && !method.IsGenericMethod) {
							ParameterInfo[] parameterInfos = method.GetParameters();

							if (parameterInfos.Length <= 1) {
								if (parameterInfos.Length == 1 && parameterInfos[0].ParameterType != typeof(ObjectReference))
									continue;

								string name = type.FullName + "." + method.Name;

								userFunctions.Add(name.GetHashCode(StringComparison.Ordinal), GetFunctionPointer(method));
							}
						}
					}
				}
			}

			GC.Collect();
			GC.WaitForPendingFinalizers();

			return userFunctions;
		}
      
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
		private static string GetTypeName(Type type) => type.FullName.Replace(".", string.Empty, StringComparison.Ordinal);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private static string GetMethodName(Type[] parameters, Type returnType) {
			string name = GetTypeName(returnType);

			foreach (Type type in parameters) {
				name += '_' + GetTypeName(type);
			}

			return name;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private static Type GetDelegateType(Type[] parameters, Type returnType) {
			string methodName = GetMethodName(parameters, returnType);

			return delegateTypesCache.GetOrAdd(methodName, () => MakeDelegate(parameters, returnType, methodName));
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private static Type MakeDelegate(Type[] types, Type returnType, string name) {
			TypeBuilder builder = moduleBuilder.DefineType(name, delegateTypeAttributes, typeof(MulticastDelegate));

			builder.DefineConstructor(ctorAttributes, CallingConventions.Standard, delegateCtorSignature).SetImplementationFlags(implAttributes);
			builder.DefineMethod("Invoke", invokeAttributes, returnType, types).SetImplementationFlags(implAttributes);

			return builder.CreateTypeInfo();
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private static IntPtr GetFunctionPointer(MethodInfo method) {
			string methodName = $"{ method.DeclaringType.FullName }.{ method.Name }";

			Delegate dynamicDelegate = delegatesCache.GetOrAdd(methodName, () => {
				ParameterInfo[] parameterInfos = method.GetParameters();
				Type[] parameterTypes = new Type[parameterInfos.Length];

				for (int i = 0; i < parameterTypes.Length; i++) {
					parameterTypes[i] = parameterInfos[i].ParameterType;
				}

				return method.CreateDelegate(GetDelegateType(parameterTypes, method.ReturnType));
			});

			return Collector.GetFunctionPointer(dynamicDelegate);
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	partial struct LinearColor {
		private float r;
		private float g;
		private float b;
		private float a;
	}

	[StructLayout(LayoutKind.Sequential)]
	partial struct Transform {
		private Vector3 location;
		private Quaternion rotation;
		private Vector3 scale;
	}


	[StructLayout(LayoutKind.Explicit, Size = 28)]
	partial struct Bounds {
		[FieldOffset(0)]
		private Vector3 origin;
		[FieldOffset(12)]
		private Vector3 boxExtent;
		[FieldOffset(24)]
		private float sphereRadius;
	}

	internal struct Bool {
		private byte value;

		public Bool(byte value) => this.value = value;

		public static implicit operator bool(Bool value) => value.value != 0;

		public static implicit operator Bool(bool value) => !value ? new(0) : new(1);

		public override int GetHashCode() => value.GetHashCode();
	}


	static unsafe partial class Assert {
		internal static delegate* unmanaged[Cdecl]<byte[], void> outputMessage;
	}

    static unsafe partial class Debug {
		internal static delegate* unmanaged[Cdecl]<LogLevel, byte[], void> log;
		internal static delegate* unmanaged[Cdecl]<byte[], void> exception;
		internal static delegate* unmanaged[Cdecl]<int, float, int, byte[], void> addOnScreenMessage;
		internal static delegate* unmanaged[Cdecl]<void> clearOnScreenMessages;
		internal static delegate* unmanaged[Cdecl]<in Vector3, in Vector3, in Quaternion, int, Bool, float, byte, float, void> drawBox;
		internal static delegate* unmanaged[Cdecl]<in Vector3, float, float, in Quaternion, int, Bool, float, byte, float, void> drawCapsule;
		internal static delegate* unmanaged[Cdecl]<in Vector3, in Vector3, float, float, float, int, int, Bool, float, byte, float, void> drawCone;
		internal static delegate* unmanaged[Cdecl]<in Vector3, in Vector3, float, int, int, Bool, float, byte, float, void> drawCylinder;
		internal static delegate* unmanaged[Cdecl]<in Vector3, float, int, int, Bool, float, byte, float, void> drawSphere;
		internal static delegate* unmanaged[Cdecl]<in Vector3, in Vector3, int, Bool, float, byte, float, void> drawLine;
		internal static delegate* unmanaged[Cdecl]<in Vector3, float, int, Bool, float, byte, void> drawPoint;
		internal static delegate* unmanaged[Cdecl]<void> flushPersistentLines;
	}

	internal static unsafe class Object {
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], void> getName;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref bool, Bool> getBool;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref byte, Bool> getByte;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref short, Bool> getShort;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref int, Bool> getInt;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref long, Bool> getLong;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref ushort, Bool> getUShort;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref uint, Bool> getUInt;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref ulong, Bool> getULong;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref float, Bool> getFloat;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref double, Bool> getDouble;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ref int, Bool> getEnum;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], byte[], Bool> getString;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], byte[], Bool> getText;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], Bool, Bool> setBool;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], byte, Bool> setByte;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], short, Bool> setShort;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], int, Bool> setInt;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], long, Bool> setLong;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ushort, Bool> setUShort;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], uint, Bool> setUInt;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], ulong, Bool> setULong;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], float, Bool> setFloat;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], double, Bool> setDouble;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], int, Bool> setEnum;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], byte[], Bool> setString;
		internal static delegate* unmanaged[Cdecl]<IntPtr, byte[], byte[], Bool> setText;
	}
	
}
