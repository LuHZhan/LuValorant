下面是一个完整的示例，展示如何触发 GMP 消息系统的递归警告：

```cpp
// 定义消息标识
#define MSG_RECURSIVE TEXT("Test.Recursive")

UCLASS()
class ARecursiveTest : public AActor
{
    GENERATED_BODY()

public:
    ARecursiveTest()
    {
        // 监听消息
        FGMPHelper::ListenMessage(MSG_RECURSIVE, this, 
            [this](const FString& Message, int32 Count)
            {
                UE_LOG(LogTemp, Warning, TEXT("收到消息: %s, 计数: %d"), *Message, Count);
                
                // 如果计数大于0，继续发送消息，这会触发递归
                if(Count > 0)
                {
                    // 在消息处理过程中再次发送相同消息
                    FGMPHelper::SendMessage(MSG_RECURSIVE, 
                        FString::Printf(TEXT("递归层级 %d"), Count), 
                        Count - 1);
                }
            });
    }

    void TriggerRecursive()
    {
        // 触发初始消息
        FGMPHelper::SendMessage(MSG_RECURSIVE, TEXT("开始递归"), 5);
    }
};
```

这个例子会产生以下行为：

1. 首次调用 `TriggerRecursive()`
2. 发送消息 `MSG_RECURSIVE` 带参数 ("开始递归", 5)
3. 消息处理函数被调用
4. 处理函数中再次发送 `MSG_RECURSIVE` 带参数 ("递归层级 5", 4)
5. 重复步骤3-4直到计数为0

在编辑器中会看到：
1. 递归警告日志：`Recursion Detected!`
2. 消息调用历史记录
3. 每层递归的日志输出

调用链如下：
```
SendMessage("开始递归", 5)
  -> OnMessage -> SendMessage("递归层级 5", 4)
    -> OnMessage -> SendMessage("递归层级 4", 3)
      -> OnMessage -> SendMessage("递归层级 3", 2)
        -> OnMessage -> SendMessage("递归层级 2", 1)
          -> OnMessage -> SendMessage("递归层级 1", 0)
```

这个示例展示了：
1. 如何触发递归消息
2. GMP的递归检测机制
3. 消息历史记录功能
4. 安全的递归终止条件