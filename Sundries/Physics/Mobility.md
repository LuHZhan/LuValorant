# UE5中ActorTransform的Mobility类型解析

在虚幻引擎5中，`Mobility`是一个非常重要的属性，它定义了Actor或Component在游戏世界中的移动能力。从当前仓库的代码分析，我们可以看到`Mobility`主要在`EComponentMobility`枚举中定义，并在多个地方被使用。

## Mobility的类型及其用途

`EComponentMobility`枚举定义了三种主要类型：

### 1. Static（静态）

```cpp
EComponentMobility::Static
```

**特点：**
- 完全不可移动的组件
- 在游戏运行时不能改变位置、旋转或缩放
- 可以在编辑器中手动移动，但一旦游戏开始运行就固定不变
- 静态网格体可以合并为一个大的网格体，减少绘制调用
- 可以使用静态光照贴图，预计算光照信息

**适用场景：**
- 建筑物、地形、不需要移动的装饰物
- 需要最高性能和最佳光照质量的场景元素

### 2. Stationary（固定）

```cpp
EComponentMobility::Stationary
```

**特点：**
- 可以改变材质或其他属性，但不能移动、旋转或缩放
- 位于静态和可移动之间的中间状态
- 可以使用混合光照技术（部分预计算，部分实时）
- 比静态组件消耗更多资源，但比可移动组件更高效

**适用场景：**
- 需要改变外观但不需要移动的物体
- 需要在游戏运行时开关的灯光
- 需要一定程度光照质量但又有一些动态特性的物体

### 3. Movable（可移动）

```cpp
EComponentMobility::Movable
```

**特点：**
- 可以在游戏运行时自由移动、旋转和缩放
- 使用完全动态光照
- 性能消耗最高
- 不能使用静态光照贴图，光照质量可能较低
- 可以参与物理模拟

**适用场景：**
- 角色、载具、武器等需要移动的物体
- 需要物理模拟的物体
- 需要在游戏中被玩家或AI交互并移动的物体

## 在代码中的应用

从当前仓库的代码片段中，我们可以看到Mobility的几个重要应用：

1. 在`DMXMVRSceneActor.cpp`中设置组件的移动性：

```cpp
RootComponentOfChildActor->SetMobility(EComponentMobility::Movable);
```

2. 在`PrimitiveSceneDesc.h`中获取移动性：

```cpp
EComponentMobility::Type GetMobility() { return Mobility; }
```

3. 在视图相关性计算中考虑移动性：

```cpp
FPrimitiveViewRelevance FTextRenderSceneProxy::GetViewRelevance(const FSceneView* View) const
{
    // ... 移动性会影响渲染决策
}
```

## Mobility对性能的影响

1. **渲染性能**：
   - Static组件可以批处理，减少绘制调用
   - Movable组件需要每帧更新变换矩阵，增加CPU负担

2. **光照性能**：
   - Static组件可以使用预计算光照，减少运行时计算
   - Movable组件需要实时光照计算，增加GPU负担

3. **物理性能**：
   - Static组件通常不参与物理模拟
   - Movable组件可能需要物理模拟，增加物理引擎负担

## 最佳实践

1. 尽可能使用Static类型，特别是对于永远不会移动的大型环境物体
2. 对于需要某些变化但不需要移动的物体，使用Stationary
3. 只对真正需要移动的物体使用Movable类型
4. 在性能敏感的场景中，谨慎使用Movable类型的光源

通过合理设置Mobility属性，可以在视觉质量和性能之间取得良好的平衡，这对于优化游戏性能至关重要。