# 客户端拥有但服务器生成的Actor是否能调用RPC

在UE5中，当服务器生成一个Actor并将其Owner设置为客户端的PlayerController时，这个Actor确实可以调用RPC（远程过程调用）。

## 为什么可以调用RPC

根据当前代码库中的实现，这种情况下Actor可以调用RPC的原因如下：

1. **HasNetOwner()和HasLocalNetOwner()函数**：
   
   在Actor.h中可以看到这两个关键函数：
   
   ```cpp
   /**
    * Does this actor have an owner responsible for replication? (APlayerController typically)
    *
    * @return true if this actor can call RPCs or false if no such owner chain exists
    */
   ENGINE_API virtual bool HasNetOwner() const;
   
   /**
    * Does this actor have a locally controlled owner responsible for replication? (APlayerController typically)
    *
    * @return true if this actor can call RPCs or false if no such owner chain exists
    */
   ENGINE_API virtual bool HasLocalNetOwner() const;
   ```
   
   从注释中可以明确看出，当Actor有一个负责复制的所有者（通常是PlayerController）时，它可以调用RPC。

2. **所有权链（Ownership Chain）**：
   
   当你将Actor的Owner设置为客户端的PlayerController时，你建立了一个所有权链。在IsOwnedBy函数中可以看到这种链式关系的检查：
   
   ```cpp
   inline bool IsOwnedBy(const AActor* TestOwner) const
   {
       for(const AActor* Arg=this; Arg; Arg=Arg->Owner)
       {
           if(Arg == TestOwner)
               return true;
       }
       return false;
   }
   ```

3. **网络权限传递**：
   
   当PlayerController在客户端上是本地控制的（LocalRole为ROLE_AutonomousProxy），它的所有权会传递给它拥有的Actor，使这些Actor能够在客户端上执行RPC调用，即使这些Actor是由服务器生成的。

## 实际工作原理

当服务器生成一个Actor并设置其Owner为客户端的PlayerController时：

1. 服务器上，这个Actor的LocalRole为ROLE_Authority，RemoteRole为ROLE_SimulatedProxy
2. 当Actor复制到客户端时，角色会交换：客户端上Actor的LocalRole为ROLE_SimulatedProxy，RemoteRole为ROLE_Authority
3. 但是，由于Actor的Owner是客户端控制的PlayerController，它被视为"客户端拥有"
4. 当检查是否可以执行RPC时，引擎会检查Actor是否有本地网络所有者（HasLocalNetOwner()）

在PackageMapClient.cpp中的SerializeNewActor函数和其他网络相关代码中，可以看到引擎如何处理Actor的网络所有权和RPC权限。

## 总结

服务器生成并将Owner设置为客户端PlayerController的Actor可以调用RPC，因为：

1. 它建立了一个到本地控制的PlayerController的所有权链
2. 引擎在决定是否允许RPC调用时会检查这种所有权关系
3. 这种模式在UE5中是常见且有效的网络架构模式，允许服务器创建由特定客户端控制的对象

这种模式在多人游戏中非常有用，可以让服务器保持对游戏对象创建的控制，同时允许客户端对其"拥有"的对象进行操作。

### -----------------------------

# 客户端勾选bReplicate无法正常创建的原因解析

在UE5中，当你在客户端上创建一个勾选了`bReplicates`的Actor时，确实会遇到问题。这与UE的网络复制模型有关，特别是与RPC调用权限相关。

## 为什么客户端无法创建勾选了bReplicates的Actor

从当前代码库中可以看到，在`AActor::PostSpawnInitialize`函数中有以下关键代码：

```cpp
// Set network role.
check(GetLocalRole() == ROLE_Authority);
ExchangeNetRoles(bRemoteOwned);
```

这段代码检查新创建的Actor的`LocalRole`是否为`ROLE_Authority`。在Actor创建时，默认情况下`LocalRole`总是被设置为`ROLE_Authority`。

当一个Actor被标记为`bReplicates = true`时，在客户端上创建该Actor会导致问题，因为：

1. 客户端创建的Actor初始`LocalRole`为`ROLE_Authority`

2. 但在网络游戏中，客户端不应该拥有`ROLE_Authority`的Actor（除非是特殊情况）

3. **当这个Actor尝试复制时，会发生角色冲突，因为服务器才是真正的权威**

### Actor复制时的角色冲突解析

   在虚幻引擎的网络模型中，当客户端创建一个设置了`bReplicates = true`的Actor时，会发生角色冲突。这个冲突具体指的是网络角色(NetRole)的冲突。

   #### 角色冲突的具体原因

   从当前代码库中，我们可以看到在`AActor::PostSpawnInitialize`函数中有以下关键代码：

   ```cpp
   // Set network role.
   check(GetLocalRole() == ROLE_Authority);
   ExchangeNetRoles(bRemoteOwned);
   ```

   这段代码首先检查新创建的Actor的`LocalRole`是否为`ROLE_Authority`，然后根据`bRemoteOwned`参数决定是否交换`Role`和`RemoteRole`。

   当我们查看`ExchangeNetRoles`函数的实现：

   ```cpp
   void AActor::ExchangeNetRoles(bool bRemoteOwned)
   {
       checkf(!HasAnyFlags(RF_ClassDefaultObject), TEXT("ExchangeNetRoles should never be called on a CDO as it causes issues when replicating actors over the network due to mutated transient data!"));
   
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

   角色冲突具体发生在以下几个方面：

   1. **初始角色分配**：
      - 当Actor在客户端创建时，默认`LocalRole`被设置为`ROLE_Authority`
      - 当设置`bReplicates = true`时，`RemoteRole`被设置为`ROLE_SimulatedProxy`
      - 这意味着客户端认为它对这个Actor有权威控制权

   2. **网络复制时的冲突**：
      - 在网络游戏中，服务器应该是唯一拥有`ROLE_Authority`的实体
      - 当客户端创建的可复制Actor尝试与服务器通信时，服务器也认为自己对该Actor有`ROLE_Authority`
      - 这导致了两个不同的网络节点都声称对同一个Actor有权威控制权

   3. **复制系统的混乱**：
      - 在`SetReplicates`函数中，我们可以看到当`bReplicates`设置为true时，会调用`MyWorld->AddNetworkActor(this)`
      - 这会将Actor添加到网络Actor列表中，使其参与网络复制
      - 当客户端和服务器都认为自己是权威时，会导致复制系统混乱，可能导致状态不一致或网络错误

   #### 代码中的相关实现

   从`AActor::SetReplicates`函数中可以看到：

   ```cpp
   void AActor::SetReplicates(bool bInReplicates)
   { 
       // Due to SetRemoteRoleForBackwardsCompat, it's possible that bReplicates is false, but RemoteRole is something other than ROLE_None.
       // So, we'll also make sure that we don't need to update RemoteRole here, even if not bReplicates wouldn't change, to fix up that case.
       const ENetRole ExpectedRemoteRole = bInReplicates ? ROLE_SimulatedProxy : ROLE_None;
   
       if (GetLocalRole() != ROLE_Authority)
       {
           UE_LOG(LogActor, Warning, TEXT("SetReplicates called on actor '%s' that is not valid for having its role modified."), *GetName());
       }
       else if (bReplicates != bInReplicates || RemoteRole != ExpectedRemoteRole) 
       {
           // ...设置复制相关的代码...
       }
   }
   ```

   这段代码明确检查了`GetLocalRole() != ROLE_Authority`的情况，并发出警告。这表明只有拥有权威角色的Actor才应该设置复制属性。

   #### 解决方案

   为了避免这种角色冲突，正确的做法是：

   1. 在服务器上创建需要复制的Actor
   2. 服务器会自动将Actor复制到客户端
   3. 如果需要客户端控制，可以通过设置Owner或使用`SetAutonomousProxy`来实现

   这样可以确保网络角色分配正确，避免复制过程中的冲突。

### -----------------------------

## "客户端必须拥有调用RPC的Actor"的含义

这句话的含义是：

1. **所有权概念**：在UE网络模型中，"拥有"一个Actor意味着客户端对该Actor有一定程度的控制权

2. **RPC调用权限**：
   - 客户端只能在它"拥有"的Actor上调用Server RPC（从客户端到服务器的RPC）
   - 这是通过`GetFunctionCallspace`函数实现的，该函数决定RPC是在本地执行还是发送到远程机器

3. **网络角色决定所有权**：
   - 如果Actor的`LocalRole`是`ROLE_Authority`，它在服务器上是权威的
   - 如果Actor的`LocalRole`是`ROLE_AutonomousProxy`，它在客户端上是可控的

从`AActor::ExchangeNetRoles`函数可以看到：

```cpp
void AActor::ExchangeNetRoles(bool bRemoteOwned)
{
    checkf(!HasAnyFlags(RF_ClassDefaultObject), TEXT("ExchangeNetRoles should never be called on a CDO as it causes issues when replicating actors over the network due to mutated transient data!"));

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

这个函数在Actor初始化时交换`Role`和`RemoteRole`，但只有当`bRemoteOwned`为true时才会执行交换。

## 正确的做法

要在客户端创建Actor并使其能够调用RPC，有几种方法：

1. **不勾选bReplicates**：如果Actor只在客户端存在，不需要复制到服务器，可以不勾选`bReplicates`

2. **使用SpawnActorDeferred**：在服务器上创建Actor，然后设置其Owner为客户端的PlayerController，这样客户端就"拥有"了这个Actor

3. **使用SetAutonomousProxy**：如果需要客户端控制的Actor，可以在服务器上创建后调用`SetAutonomousProxy(true)`，将其RemoteRole设置为`ROLE_AutonomousProxy`

4. **使用RPC请求服务器创建**：客户端不直接创建复制的Actor，而是发送RPC请求服务器创建，并设置适当的所有权

## 总结

"客户端必须拥有调用RPC的Actor"意味着：只有当客户端对Actor有控制权（通常通过所有权链或自主代理角色实现）时，才能从该Actor调用Server RPC。这是UE网络安全模型的一部分，防止客户端对它们不应该控制的Actor执行操作。

在实际开发中，通常的做法是让服务器创建所有需要复制的Actor，然后根据需要将某些Actor的控制权（通过设置Owner或角色）分配给特定客户端。