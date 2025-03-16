在 Unreal Engine 4（UE4）中，网络同步是多人游戏开发的核心机制，涉及 **角色权限（Role）**、**属性同步（Replication）** 和 **RPC（Remote Procedure Call）** 三大核心功能。以下是它们的详细说明及优劣对比：

---

### **1. 角色权限（Role）**
UE4 中的每个 Actor 在网络中都有一个 `Role`，决定了其在网络环境中的控制权和同步权限。  
角色类型分为以下三种：

#### **(1) `ROLE_Authority`（权威角色）**
- **归属**：仅存在于 **服务器**（Server）。
- **权限**：拥有对 Actor 的完全控制权，负责逻辑计算、状态同步和合法性验证。
- **典型场景**：服务器上的 NPC、游戏规则管理器、玩家控制的角色（服务器端副本）。

#### **(2) `ROLE_AutonomousProxy`（自主代理）**
- **归属**：**客户端**（Client）控制的 Actor（如玩家控制的角色）。
- **权限**：客户端可以发起操作请求（如移动、技能释放），但最终由服务器验证和执行。
- **典型场景**：玩家角色的本地控制（客户端预测移动）。

#### **(3) `ROLE_SimulatedProxy`（模拟代理）**
- **归属**：**其他客户端** 或服务器上非控制的 Actor。
- **权限**：仅同步服务器状态，无法发起操作。
- **典型场景**：其他玩家的角色、服务器控制的 NPC（客户端视角）。

---

### **2. 属性同步（Replication）**
属性同步通过标记 `Replicated` 实现自动状态同步，确保客户端与服务器数据一致。

#### **(1) 实现方式**
- **标记复制属性**：  
  在变量声明中添加 `UPROPERTY(Replicated)`：
  ```cpp
  UPROPERTY(Replicated)
  int32 Health;
  ```
- **重写 `GetLifetimeReplicatedProps`**：  
  指定需要同步的变量：
  ```cpp
  void AMyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
      Super::GetLifetimeReplicatedProps(OutLifetimeProps);
      DOREPLIFETIME(AMyActor, Health);
  }
  ```
- **条件复制**（可选）：  
  使用 `DOREPLIFETIME_CONDITION` 按条件同步（如仅对 Owner 同步）。

#### **(2) 同步触发机制**
- **自动同步**：服务器修改属性后，自动同步到客户端。
- **回调通知**（`RepNotify`）：  
  属性变化时触发回调函数：
  ```cpp
  UPROPERTY(ReplicatedUsing = OnRep_Health)
  int32 Health;
  
  UFUNCTION()
  void OnRep_Health();
  ```

#### **(3) 优劣**
| **优点**                     | **缺点**                       |
| ---------------------------- | ------------------------------ |
| 自动同步，减少代码量         | 高频同步可能导致带宽压力       |
| 支持差值同步（`RepNotify`）  | 延迟较高（依赖服务器更新频率） |
| 适合连续状态（如位置、血量） | 无法精确控制同步时机           |

---

### **3. RPC（Remote Procedure Call）**
RPC 用于在客户端和服务器之间直接触发特定函数。

#### **(1) RPC 类型**
| **类型**         | **触发方** | **执行方**               | **用途**                         |
| ---------------- | ---------- | ------------------------ | -------------------------------- |
| **Server**       | 客户端     | 服务器                   | 请求服务器执行敏感操作（如开火） |
| **Client**       | 服务器     | 指定客户端               | 通知客户端播放特效、更新UI       |
| **NetMulticast** | 服务器     | 所有客户端（包括服务器） | 全局事件（如爆炸特效、游戏结束） |

#### **(2) 实现方式**
- **声明 RPC**：使用 `UFUNCTION` 宏标记：
  ```cpp
  // Server RPC（需验证函数）
  UFUNCTION(Server, Reliable, WithValidation)
  void Server_Fire();
  
  bool Server_Fire_Validate() { return true; } // 参数验证
  void Server_Fire_Implementation() { /* 逻辑 */ }
  ```
- **触发 RPC**：  
  - `Server` RPC：客户端调用，服务器执行。
  - `Client`/`NetMulticast` RPC：服务器调用，客户端执行。

#### **(3) 优劣**
| **优点**                     | **缺点**                        |
| ---------------------------- | ------------------------------- |
| 实时性强，精准控制逻辑       | 需要手动管理网络权限和触发时机  |
| 适合一次性事件（如技能释放） | 高频调用可能增加延迟和丢包率    |
| 支持参数传递和返回值（单播） | 不可靠（Unreliable）RPC可能丢失 |

---

### **4. 对比总结**
| **机制**     | **适用场景**                 | **性能影响**       | **控制粒度** |
| ------------ | ---------------------------- | ------------------ | ------------ |
| **属性同步** | 连续状态（位置、血量、弹药） | 中（依赖更新频率） | 粗粒度       |
| **RPC**      | 离散事件（开火、技能、交互） | 高（频繁调用时）   | 细粒度       |

---

### **5. 最佳实践**
1. **混合使用**：  
   - 用 **属性同步** 同步基础状态（如位置、血量）。  
   - 用 **RPC** 处理即时事件（如技能释放、UI交互）。
2. **优化带宽**：  
   - 对高频属性（如位置）使用压缩或差值同步（`RepNotify`）。  
   - 对非关键 RPC 使用 `Unreliable`（如音效触发）。
3. **防作弊**：  
   - 所有客户端请求必须通过 **Server RPC** 并在服务器验证。  
   - 使用 `_Validate` 函数检查参数合法性。
4. **客户端预测**：  
   - 对玩家移动等操作使用客户端预测 + 服务器校正（通过属性同步）。

---

### **6. 注意事项**
- **网络角色检查**：  
  在敏感逻辑前检查 `Role == ROLE_Authority`，避免客户端篡改数据。
- **Actor 复制条件**：  
  确保 Actor 的 `bReplicates = true`，否则属性和 RPC 无法同步。
- **延迟补偿**：  
  对射击判定等操作使用服务器回溯（Lag Compensation）。

通过合理结合属性同步和 RPC，可以在保证游戏实时性的同时，有效控制网络带宽和安全性。

### -----------------------------

在 Unreal Engine 5 中，当服务器生成一个 **已启用网络同步的 Actor**，并在客户端因性能问题延迟生成该 Actor 的期间修改了其属性 `x`，**客户端最终生成的 Actor 是否能同步到最新的 `x` 值**，取决于以下关键机制：

---

### **1. UE5 网络同步的核心流程**
- **服务器生成 Actor**：  
  服务器生成 Actor 时，会记录其初始属性值（包括 `x` 的初始值）。
- **初始同步（Initial Replication）**：  
  服务器通过 **网络更新周期** 将 Actor 的创建信息和初始属性打包发送给客户端。  
  **关键点**：初始同步数据包含的是 Actor 生成时所有 `Replicated` 属性的 **当前值**。
- **后续属性修改**：  
  服务器在生成后对 `x` 的修改会触发 **增量同步**（后续的 `Replicated` 更新）。

---

### **2. 客户端延迟生成的影响**
如果客户端因性能问题延迟生成 Actor，可能出现以下两种场景：

#### **场景 1：服务器修改 `x` 发生在初始同步数据发送前**
- **同步结果**：  
  客户端生成的 Actor 会直接接收到修改后的 `x` 值。  
- **原因**：  
  UE 的初始同步数据是在 **网络更新周期结束时** 统一收集并发送的。如果服务器在同一帧内生成 Actor 并修改 `x`，初始同步数据会包含最新的 `x` 值。

#### **场景 2：服务器修改 `x` 发生在初始同步数据发送后**
- **同步结果**：  
  客户端生成的 Actor 会先接收到初始值，随后再通过 **增量同步** 更新到最新的 `x` 值。  
- **原因**：  
  初始同步数据已发送旧值，后续修改会作为增量更新发送。客户端在生成 Actor 后，会立即处理后续的增量同步。

---

### **3. 验证实验**
可以通过以下实验验证逻辑：
1. **服务器代码**：
   ```cpp
   // 生成 Actor
   AActor* NewActor = GetWorld()->SpawnActor<AActor>(...);
   NewActor->x = 100; // 初始值
   
   // 立即修改 x（同一帧）
   NewActor->x = 200;
   ```
2. **客户端观察结果**：  
   - 若客户端生成的 Actor 的 `x` 值为 200，则符合 **场景 1**。  
   - 若客户端先生成 `x=100`，随后更新到 `x=200`，则符合 **场景 2**。

---

### **4. 同步机制总结**
| **服务器操作时序**              | **客户端最终同步的 `x` 值**                    |
| ------------------------------- | ---------------------------------------------- |
| 生成 Actor → 修改 `x`（同一帧） | 直接同步最新值（200）                          |
| 生成 Actor → 下一帧修改 `x`     | 先同步初始值（100），再增量同步到最新值（200） |

---

### **5. 注意事项**
- **网络更新周期**：  
  UE 默认的网络更新频率为每秒 100 次（可通过 `NetUpdateFrequency` 调整）。高频修改需注意带宽消耗。
- **属性同步优先级**：  
  使用 `ReplicatedUsing` 和 `RepNotify` 回调处理关键属性变化：
  ```cpp
  UPROPERTY(ReplicatedUsing = OnRep_X)
  int32 x;
  
  UFUNCTION()
  void OnRep_X() {
      // 客户端收到 x 的更新
  }
  ```
- **客户端生成延迟的优化**：  
  若客户端生成延迟过高，可能导致增量同步数据丢失（UE 默认不缓存未生成 Actor 的更新）。可通过以下方式缓解：  
  - 提高客户端性能（优化代码或降低负载）。  
  - 使用 `DOREPLIFETIME_CONDITION` 限制非必要属性的同步频率。

---

### **结论**
- **可以同步**：无论服务器何时修改 `x`，只要属性标记为 `Replicated`，客户端最终会同步到最新值。  
- **同步时序**：  
  - 若修改发生在初始同步数据发送前，客户端直接获得最新值。  
  - 若修改发生在初始同步数据发送后，客户端通过增量更新获得最新值。  

确保合理设计网络同步逻辑，避免高频修改关键属性导致性能问题。



### -----------------------------



# UE5中UPROPERTY的Replicated属性同步机制

根据当前UE5.4引擎代码库，UPROPERTY的Replicated属性同步确实是固定从服务器同步到客户端的，这是UE网络架构的基本设计原则。

## 同步方向

从`Actor.h`中可以看到，网络复制系统遵循以下规则：

1. 数据总是从服务器（Authority，即`ROLE_Authority`）同步到客户端
2. 客户端无法直接修改其他客户端的数据
3. 服务器是唯一的权威来源

例如，在`Actor.h`中可以看到：

```cpp
/** If true, this actor will replicate to network clients */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Replication)
uint8 bReplicates:1;
```

## 同步频率

关于同步频率，UE5提供了几个关键参数来控制：

1. **NetUpdateFrequency**：
   ```cpp
   /** How often (per second) this actor will be considered for replication, used to determine NetUpdateTime */
   UPROPERTY(Category=Replication, EditDefaultsOnly, BlueprintReadWrite)
   float NetUpdateFrequency;
   ```
   这个属性定义了每秒钟Actor被考虑进行复制的次数。默认值通常为100Hz，但实际上引擎会根据网络负载和优先级进行调整。

2. **MinNetUpdateFrequency**：
   ```cpp
   /** Used to determine what rate to throttle down to when replicated properties are changing infrequently */
   UPROPERTY(Category=Replication, EditDefaultsOnly, BlueprintReadWrite)
   float MinNetUpdateFrequency;
   ```
   当属性变化不频繁时，引擎会降低到这个频率来节省带宽。

3. **NetPriority**：
   ```cpp
   /** Priority for this actor when checking for replication in a low bandwidth or saturated situation, higher priority means it is more likely to replicate */
   UPROPERTY(Category=Replication, EditDefaultsOnly, BlueprintReadWrite)
   float NetPriority;
   ```
   在网络带宽有限的情况下，决定哪些Actor优先同步。

## 实际同步频率的决定因素

实际的同步频率受多种因素影响：

1. **网络带宽**：当带宽受限时，引擎会优先同步高优先级的Actor
2. **相关性**：只有与客户端相关的Actor才会被同步
3. **属性变化**：只有当属性值发生变化时才会同步
4. **网络拥塞控制**：引擎会根据网络状况动态调整同步频率

## 优化同步的方法

UE5提供了多种方式来优化网络同步：

1. **条件复制**：使用DOREPLIFETIME_CONDITION来设置复制条件
   ```cpp
   DOREPLIFETIME_CONDITION(AYourActor, YourProperty, COND_OwnerOnly);
   ```

2. **网络相关性**：通过重写IsNetRelevantFor函数来控制Actor对特定客户端的相关性

3. **网络休眠**：使用NetDormancy来控制Actor的休眠状态
   ```cpp
   /** Dormancy setting for actor to take itself off of the replication list without being destroyed on clients. */
   UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Replication)
   TEnumAsByte<enum ENetDormancy> NetDormancy;
   ```

4. **自定义复制**：通过重写GetLifetimeReplicatedProps来精细控制属性复制

## 总结

在UE5中，Replicated属性的同步：
- 固定从服务器同步到客户端
- 基础频率由NetUpdateFrequency决定（默认约为100Hz）
- 实际频率受网络条件、优先级和相关性等多种因素影响
- 可以通过多种机制进行优化和控制

这种设计确保了游戏状态的一致性，同时提供了足够的灵活性来优化网络性能。

### -----------------------------



在 Unreal Engine 5 中，能够调用 **RPC（Remote Procedure Call）** 的 Actor 需要满足特定的 **网络权限** 和 **所有权** 条件。以下是不同 RPC 类型对 Actor 的要求及调用规则：

---

### **1. RPC 类型与调用规则**
| **RPC 类型**         | **触发方** | **执行方**               | **调用条件**                                                 |
| -------------------- | ---------- | ------------------------ | ------------------------------------------------------------ |
| **Server RPC**       | 客户端     | 服务器                   | Actor 必须由客户端控制（`ROLE_AutonomousProxy`），或明确设置网络所有权。 |
| **Client RPC**       | 服务器     | 指定客户端               | Actor 必须由服务器生成（`ROLE_Authority`），且客户端拥有该 Actor 的所有权。 |
| **NetMulticast RPC** | 服务器     | 所有客户端（包括服务器） | Actor 必须由服务器生成（`ROLE_Authority`），且无需明确所有权。 |

---

### **2. 可调用 RPC 的 Actor 类型**
#### **(1) 客户端控制的 Actor**
- **PlayerController（客户端拥有）**  
  - **网络角色**：`ROLE_AutonomousProxy`（客户端）或 `ROLE_Authority`（服务器端副本）。  
  - **权限**：客户端 PlayerController 可以调用 **Server RPC**（如玩家输入事件）。  
  - **示例**：  
    
    ```cpp
    // 客户端触发开火请求
    UFUNCTION(Server, Reliable)
    void Server_Fire();
    ```
  
- **客户端控制的 Pawn/Character**  
  - **条件**：被客户端 `Possess` 或 `Spawn` 时指定 Owner 为客户端 PlayerController。  
  - **权限**：可调用 **Server RPC**（如跳跃、技能释放）。  
  - **示例**：  
    ```cpp
    // 客户端控制角色触发移动
    UFUNCTION(Server, Reliable)
    void Server_Jump();
    ```

#### **(2) 服务器生成的 Actor**
- **服务器权威 Actor**  
  - **网络角色**：`ROLE_Authority`（仅存在于服务器）。  
  - **权限**：可调用 **Client RPC** 和 **NetMulticast RPC**。  
  - **示例**：  
    ```cpp
    // 服务器触发的全局爆炸特效
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Explode();
    ```

- **客户端拥有但服务器生成的 Actor**  
  - **条件**：生成时指定 Owner 为客户端 PlayerController（如 `SpawnActor` 中设置 `Owner`）。  
  - **权限**：可调用 **Client RPC**（仅通知该客户端）。  
  - **示例**：  
    ```cpp
    // 服务器通知客户端显示 UI
    UFUNCTION(Client, Reliable)
    void Client_ShowDamageUI();
    ```

---

### **3. 无法调用 RPC 的 Actor**
- **无所有权的 Actor**  
  - 若 Actor 未设置 `Owner`，或 Owner 不是客户端 PlayerController 或服务器，无法触发 RPC。  
  - **示例**：其他客户端的角色（`ROLE_SimulatedProxy`）无法调用 Server RPC。

- **未启用复制的 Actor**  
  - 若 Actor 的 `bReplicates` 未设置为 `true`，RPC 无法通过网络传输。

---

![img](Images/ServerRPC)

### **4. 关键验证机制**

#### **(1) 网络权限检查**
- **Server RPC**：服务器在收到客户端请求后，会检查调用者是否具有 `ROLE_AutonomousProxy` 或有效的所有权。  
- **Client/NetMulticast RPC**：服务器直接触发，客户端无条件执行。

#### **(2) 防作弊验证**
- 所有 Server RPC 必须通过 `_Validate` 函数验证参数合法性：  
  ```cpp
  bool AMyActor::Server_Fire_Validate() {
      return AmmoCount > 0; // 检查弹药是否合法
  }
  ```

#### **(3) 同步设置**
- **Actor 必须启用复制**：  
  ```cpp
  AMyActor::AMyActor() {
      bReplicates = true; // 必需！
  }
  ```

---

### **5. 示例场景**
#### **(1) 客户端触发技能（Server RPC）**
1. 客户端控制的角色调用 `Server_CastSpell()`。  
2. 服务器验证参数（如法力值是否足够）。  
3. 服务器执行逻辑并广播 `Multicast_PlaySpellEffect()`。

#### **(2) 服务器通知客户端（Client RPC）**
1. 服务器生成一个陷阱 Actor 并设置 Owner 为触发它的玩家。  
2. 陷阱触发后，服务器调用 `Client_ShowTrapWarning()` 通知该客户端。

---

### **6. 注意事项**
- **高频调用优化**：避免高频调用不可靠（`Unreliable`）RPC，可能导致数据丢失。  
- **网络延迟补偿**：对客户端预测操作（如移动）需配合服务器校正逻辑。  
- **动态生成 Actor**：动态生成的 Actor 需确保 `bReplicates = true` 且正确设置 `Owner`。

---

### **总结**
| **Actor 类型**              | **可调用的 RPC**    | **条件**                                                     |
| --------------------------- | ------------------- | ------------------------------------------------------------ |
| 客户端 PlayerController     | Server RPC          | 默认拥有 `ROLE_AutonomousProxy`                              |
| 客户端控制的 Pawn/Character | Server RPC          | Owner 为客户端 PlayerController，且 `bReplicates = true`     |
| 服务器生成的 Actor          | Client/NetMulticast | `ROLE_Authority`，且目标客户端拥有该 Actor（仅 Client RPC 需要） |

正确设置 Actor 的 **网络角色**、**所有权** 和 **复制属性**，是确保 RPC 正常工作的关键！