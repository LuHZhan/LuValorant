# UE5语法与标准C++的不同之处

UE5引擎在C++的基础上添加了许多特有的语法和宏，使其更适合游戏开发。以下是UE5中与标准C++不同的主要特性：

## 1. 反射系统和元数据

UE5使用宏系统实现了一套强大的反射机制，这是标准C++所不具备的：

```cpp
UCLASS()
class GAME_API AMyActor : public AActor
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyCategory")
    float MyProperty;
    
    UFUNCTION(BlueprintCallable, Category="MyCategory")
    void MyFunction();
};
```

这些宏（如`UCLASS`、`UPROPERTY`、`UFUNCTION`、`GENERATED_BODY`）允许引擎在编译时生成额外的元数据，支持蓝图系统、序列化和编辑器集成。

## 2. 特殊的类型系统

UE5定义了许多自己的类型，替代了标准C++类型：

```cpp
// UE5类型                  // 标准C++等价物
FString                    // std::string
TArray<T>                  // std::vector<T>
TMap<K, V>                 // std::map<K, V> 或 std::unordered_map<K, V>
TSet<T>                    // std::set<T> 或 std::unordered_set<T>
FName                      // 无直接等价物，高效的字符串标识符
FText                      // 无直接等价物，用于本地化文本
```

## 3. 内存管理

UE5有自己的内存管理系统，与标准C++的`new`/`delete`不同：

```cpp
// UE5对象创建
UMyObject* MyObj = NewObject<UMyObject>();
AActor* MyActor = GetWorld()->SpawnActor<AActor>(AMyActor::StaticClass());

// 垃圾回收而非手动删除
// 不使用delete MyObj;
```

从代码中可以看到`PostSpawnInitialize`函数中的对象创建和生命周期管理与标准C++有很大不同。

## 4. 委托和事件系统

UE5有一套复杂的委托系统，用于事件处理：

```cpp
// 声明委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageDelegate, float, Damage);

// 在类中定义
UPROPERTY(BlueprintAssignable)
FOnDamageDelegate OnDamage;

// 绑定和触发
OnDamage.AddDynamic(this, &AMyActor::HandleDamage);
OnDamage.Broadcast(10.0f);
```

## 5. 网络复制系统

UE5有特殊的网络复制语法，如在`Actor.cpp`中看到的：

```cpp
void AActor::SetReplicates(bool bInReplicates)
{ 
    const ENetRole ExpectedRemoteRole = bInReplicates ? ROLE_SimulatedProxy : ROLE_None;
    // ...
}

void AActor::ExchangeNetRoles(bool bRemoteOwned)
{
    if (!bExchangedRoles)
    {
        if (bRemoteOwned)
        {
            Exchange(Role, RemoteRole);
        }
        bExchangedRoles = true;
    }
}
```

这些网络角色和复制系统是UE5特有的，标准C++没有内置的网络功能。

## 6. 智能指针

UE5使用自己的智能指针系统：

```cpp
TSharedPtr<FMyClass> MySharedObject;  // 类似std::shared_ptr
TWeakPtr<FMyClass> MyWeakObject;      // 类似std::weak_ptr
TUniquePtr<FMyClass> MyUniqueObject;  // 类似std::unique_ptr

// UObject使用特殊的弱引用
TWeakObjectPtr<UObject> WeakObjectReference;
```

在代码中可以看到`TWeakObjectPtr<AActor>`的使用。

## 7. 特殊的预处理器宏

UE5大量使用预处理器宏来简化代码：

```cpp
// 日志宏
UE_LOG(LogActor, Warning, TEXT("SetReplicates called on actor '%s'"), *GetName());

// 检查宏
check(GetLocalRole() == ROLE_Authority);
ensure(BeginPlayCallDepth - 1 == CurrentCallDepth);

// 条件编译
#if WITH_EDITOR
// 编辑器特定代码
#endif
```

## 8. 字符串处理

UE5使用`TEXT()`宏和特殊的字符串函数：

```cpp
FString Name = TEXT("MyActor");
FString FullPath = FString::Printf(TEXT("%s_%d"), *Name, 123);
```

## 9. 引擎API导出宏

```cpp
ENGINE_API virtual bool HasNetOwner() const;
```

这些宏用于控制符号导出，使其在DLL边界上可见。

## 总结

UE5的C++扩展使其成为一种"方言"，具有强大的游戏开发特性，但也意味着UE5开发者需要学习这些特殊语法。这些扩展主要通过预处理器宏、自定义类型和特殊的编译工具链实现，使得UE5能够提供反射、序列化、网络复制和编辑器集成等游戏引擎所需的高级功能。