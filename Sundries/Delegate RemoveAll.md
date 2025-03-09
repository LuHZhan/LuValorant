# UE5 多播委托 RemoveAll 函数的作用

在虚幻引擎5的委托系统中，`RemoveAll` 函数用于从多播委托中移除所有与特定对象相关的绑定。让我们详细分析这个函数的行为。

## RemoveAll 函数的作用

`RemoveAll` 函数会删除多播委托中与指定对象相关联的所有绑定函数。具体来说：

1. 它会移除所有使用**指定对象实例**的委托绑定

   ### 未绑定到对象的委托类型
   以下类型的委托不会被 RemoveAll 函数删除：

   1. 静态函数委托 ：通过 AddStatic 或 BindStatic 绑定的全局函数或静态成员函数
      
      ```cpp
      MyDelegate.AddStatic(&SomeGlobalFunction);
      ```
   2. Lambda 委托 ：通过 AddLambda 或 BindLambda 绑定的普通 Lambda 表达式
      
      ```cpp
      MyDelegate.AddLambda([]() { /* 代码 */ });
      ```
   3. 函数对象委托 ：绑定的仿函数对象
   这些类型的委托在内部实现中没有关联到特定对象，因此 HasSameObject 方法会返回 false ，导致它们不会被 RemoveAll 函数删除。

2. 这包括通过 `AddDynamic`、`BindUObject`、`BindSP` 等方式绑定到该对象的所有函数

## 源码分析

在 `TMulticastDelegateBase` 类中（多播委托的基类），`RemoveAll` 函数的实现如下：

```cpp
/**
 * 从委托中移除与指定对象相关的所有绑定
 *
 * @param InUserObject 要移除其所有绑定的对象指针
 */
template <typename UserClass>
inline void RemoveAll(const UserClass* InUserObject)
{
    // 获取写入锁
    FWriteAccessScope WriteScope = GetWriteAccessScope();
    
    // 遍历所有委托实例
    for (int32 InvocationListIndex = InvocationList.Num() - 1; InvocationListIndex >= 0; --InvocationListIndex)
    {
        // 检查委托实例是否与指定对象绑定
        if (InvocationList[InvocationListIndex].GetDelegateInstance()->HasSameObject(InUserObject))
        {
            // 如果是，则移除该委托实例
            InvocationList.RemoveAt(InvocationListIndex);
        }
    }
}
```

## 使用场景

`RemoveAll` 函数在以下场景中特别有用：

1. **对象销毁时**：当一个对象即将被销毁时，应该调用 `RemoveAll` 来清除所有与该对象相关的委托绑定，防止悬空指针
2. **重置对象状态**：当需要重置对象的所有事件监听时
3. **避免内存泄漏**：确保不再需要的对象不会因为委托绑定而被保留在内存中

## 使用示例

```cpp
// 声明一个多播委托
DECLARE_MULTICAST_DELEGATE(FOnGameEvent);
FOnGameEvent OnGameStart;

// 在某个类中
class AMyActor : public AActor
{
public:
    void BeginPlay() override
    {
        Super::BeginPlay();
        
        // 添加委托绑定
        OnGameStart.AddUObject(this, &AMyActor::HandleGameStart);
        OnGameStart.AddUObject(this, &AMyActor::AnotherHandler);
    }
    
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override
    {
        // 移除所有与this对象相关的绑定
        OnGameStart.RemoveAll(this);
        
        Super::EndPlay(EndPlayReason);
    }
    
private:
    void HandleGameStart() { /* ... */ }
    void AnotherHandler() { /* ... */ }
};
```

## 与其他移除函数的区别

- **RemoveAll**：移除与特定对象相关的所有绑定
- **RemoveDynamic**：只移除特定对象的特定函数绑定
- **Clear**：移除所有绑定，不管是哪个对象

## 总结

`RemoveAll` 函数是一个非常有用的工具，用于在对象生命周期结束或不再需要接收事件通知时，清理与特定对象相关的所有委托绑定。正确使用这个函数可以避免悬空指针和内存泄漏问题。