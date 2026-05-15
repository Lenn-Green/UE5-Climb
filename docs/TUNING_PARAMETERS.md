# 调参文档

这份文档只记录当前适合在编辑器里手动调试的参数。

适用对象：
- [BP_ClimbingCharacter](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Content/Climb/Characters/Blueprints/BP_ClimbingCharacter.uasset>)
- [ABP_ClimbingCharacter](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Content/Climb/Characters/Animation/Blueprints/ABP_ClimbingCharacter.uasset>)

不包含：
- Control Rig 图里的节点接线本身
- C++ 只读状态变量
- 临时调试变量

## 打开方式

### BP 参数
打开 [BP_ClimbingCharacter](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Content/Climb/Characters/Blueprints/BP_ClimbingCharacter.uasset>)：
1. 点 `Class Defaults`
2. 点图里空白处
3. 在右侧 `Details` 搜参数名

### ABP 参数
打开 [ABP_ClimbingCharacter](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Content/Climb/Characters/Animation/Blueprints/ABP_ClimbingCharacter.uasset>)：
1. 点 `Class Defaults`
2. 点图里空白处，不要选 AnimGraph 节点
3. 在右侧 `Details` 搜参数名

### 组件参数
在 [BP_ClimbingCharacter](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Content/Climb/Characters/Blueprints/BP_ClimbingCharacter.uasset>) 里选中继承组件：
- `ClimbingMovement`
- `HoldQuery`

然后在右侧 `Details` 看组件参数。

---

## 一、BP_ClimbingCharacter：输入与抓取语义

来源：
- [ClimbingCharacter.h](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Source/Climb/Public/Character/ClimbingCharacter.h>)

### `ClimbingInputMappingContext`
作用：
- 当前角色用的攀爬输入映射

什么时候调：
- 只在替换整套输入资产时调

### `ClimbingInputMappingPriority`
作用：
- Enhanced Input Mapping Context 的优先级

现象：
- 如果攀爬输入被别的 Context 抢掉，可以提高这个值

### `ClimbCenterOfMassMoveAction`
### `ClimbLimbProbeAction`
### `ClimbLeftGripAction`
### `ClimbRightGripAction`
### `ClimbLeftFootGripAction`
### `ClimbRightFootGripAction`
作用：
- 输入资产绑定

现象：
- 输入完全不生效时先查这里

### `GripPressedThreshold`
作用：
- 模拟扳机/轴输入时，多少算“按下”

现象：
- 太低：轻微抖动也会被认为按下
- 太高：要按很重才有反应

建议：
- 一般保持 `0.3 ~ 0.4`

### `GripHoldLockDelaySeconds`
作用：
- 短按和长按的分界线
- 超过这个时间才尝试锁点

现象：
- 太小：本来想激活探索，却很容易误锁
- 太大：长按抓取显得迟钝

建议：
- 一般保持 `0.18 ~ 0.25`

---

## 二、BP_ClimbingCharacter：调试与探测平面

来源：
- [ClimbingCharacter.h](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Source/Climb/Public/Character/ClimbingCharacter.h>)

### `MaxCenterOfMassHorizontalOffset`
作用：
- CoM 在墙面左右方向的最大偏移范围

现象：
- 太小：重心输入显得受限
- 太大：debug 数值会变得更极端

### `MaxCenterOfMassVerticalOffset`
作用：
- CoM 在墙面上下方向的最大偏移范围

### `bDrawClimbingDebug`
作用：
- 是否画当前所有攀爬 debug

包括：
- 紫色可探索平面
- 粉色平面 target
- 黄色 candidate
- CoM 球和线

### `CenterOfMassDebugSphereRadius`
作用：
- CoM debug 球大小

---

## 三、每个肢体独立的 Reach Plane 参数

来源：
- [ClimbingCharacter.h](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Source/Climb/Public/Character/ClimbingCharacter.h>)
- `FClimbingProbePlaneSettings`

这四组参数分别独立：
- `LeftHandProbeSettings`
- `RightHandProbeSettings`
- `LeftFootProbeSettings`
- `RightFootProbeSettings`

每组参数语义一致。

### `HorizontalRange`
作用：
- 紫色椭圆的左右半径

现象：
- 太大：左手/左脚会抢到另一侧的点
- 太小：旁边的明显可达点探不到

### `VerticalRange`
作用：
- 紫色椭圆的上下半径

现象：
- 太大：会去选太高/太低的点
- 太小：本该能探到的上方或下方点探不到

### `ForwardOffset`
作用：
- 沿墙法线方向前后推整张平面
- 不是沿角色世界前方

现象：
- 太小：平面和身体重合，target 太贴身体
- 太大：平面离墙太远，抓点不自然

### `LateralOffset`
作用：
- 沿墙面左右方向偏移整张平面

现象：
- 太大：平面明显跑到 limb 外侧
- 太小：左右 limb 容易抢同一片区域

建议：
- 默认先从 `0` 开始
- 只有左右 limb 还互相抢点时再加

### `VerticalOffset`
作用：
- 沿墙面上下方向整体抬高/压低平面

现象：
- 太低：胸前手点探不到
- 太高：脚会去找不合理的高点

### `SearchDepth`
作用：
- 从 pink plane target 向墙面方向做 near-point query 的搜索深度

现象：
- 太小：平面对了但经常找不到 yellow candidate
- 太大：容易吸到侧面或后方不合理点

---

## 四、BP_ClimbingCharacter：探索与稳定性调试

### `HandExplorationReach`
作用：
- 旧版探索使用过的手部前伸距离参数

当前状态：
- 现在 `P7.B` 的主语义已经是 reach plane + near-point query
- 这个值不再是当前最核心的 plane 调参入口

建议：
- 非必要先不要动

### `StableOffsetThreshold`
作用：
- solver/debug 里判断姿势稳定的阈值

现象：
- 太小：很容易判成不稳定
- 太大：debug 过于宽松

### `MaxBodyTensionOffset`
作用：
- body tension 归一化时使用的最大参考偏移

现象：
- 太小：稍微偏一点 tension 就很高
- 太大：tension 变化不明显

---

## 五、HoldQuery 组件参数

来源：
- [ClimbingHoldQueryComponent.h](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Source/Climb/Public/Components/ClimbingHoldQueryComponent.h>)

在 [BP_ClimbingCharacter](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Content/Climb/Characters/Blueprints/BP_ClimbingCharacter.uasset>) 里选 `HoldQuery` 组件。

### `TraceDistance`
作用：
- 长距离 query 的搜索长度

现象：
- 当前 `P7.B` 主要用 near-point query，这个不是最常调的参数

### `TraceRadius`
作用：
- sphere query 半径

现象：
- 太大：附近多个岩点时容易串点
- 太小：小岩点不容易命中

建议：
- 如果“总抓到旁边的点”，优先先减这个值

### `TraceChannel`
作用：
- 用哪个碰撞通道做 hold query

现象：
- 真实岩点探不到时，先检查 mesh 是否对这个通道 `Block`

### `bTraceComplex`
作用：
- 是否使用复杂碰撞

现象：
- 真实岩点 simple collision 很粗糙时，可以试开
- 但开了以后命中表现和性能都会变

### `bDebugDraw`
作用：
- HoldQuery 自己的 trace debug

### `DebugDrawTime`
作用：
- HoldQuery debug 线保留时间

---

## 六、ClimbingMovement 组件参数

来源：
- [ClimbingMovementComponent.h](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Source/Climb/Public/Components/ClimbingMovementComponent.h>)

在 [BP_ClimbingCharacter](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Content/Climb/Characters/Blueprints/BP_ClimbingCharacter.uasset>) 里选 `ClimbingMovement` 组件。

### `TargetWallDistance`
作用：
- 角色攀爬时整体离墙的目标距离

现象：
- 太小：身体容易埋进墙
- 太大：角色像悬浮在墙外

### `AttachInterpSpeed`
作用：
- movement attachment frame 的贴合速度

现象：
- 太大：吸附快，但可能硬
- 太小：贴墙过程显得慢或拖

---

## 七、ABP_ClimbingCharacter：AnimInstance 平滑参数

来源：
- [ClimbingAnimInstance.h](</C:/Users/tangyue/Documents/Unreal Projects/Climb/Source/Climb/Public/Animation/ClimbingAnimInstance.h>)

### `PelvisOffsetInterpSpeed`
作用：
- 骨盆 offset 平滑速度

现象：
- 大：骨盆更快响应
- 小：更柔和，但会拖

### `LockedTargetInterpSpeed`
作用：
- 锁定 target 的插值速度

现象：
- 大：锁点后更快贴到最终位置
- 小：过渡更柔，但会慢半拍

### `ExplorationTargetInterpSpeed`
作用：
- exploration target 的插值速度

现象：
- 大：pink 移动时 limb 更跟手
- 小：更顺滑，但会更拖

如果现在 pink 动了但 limb 跟不紧，这个是重点参数之一。

### `TargetRotationInterpSpeed`
作用：
- target normal / rotation 的平滑速度

现象：
- 大：朝向调整更快
- 小：旋转更柔和

### `ReleaseTargetInterpSpeed`
打开位置：
- 打开 `ABP_ClimbingCharacter`
- 点 `Class Defaults`
- 搜索 `Release Target Interp Speed`
- 分类：`Climbing | Smoothing`

作用：
- limb 释放后，从最后一个 locked target 回到 neutral/input pose target 的插值速度。
- 当前 release 使用之前更顺滑的 `VInterpTo / RInterpTo` 速度型平滑。
- hand target 的 inactive/default location 仍然保持在 neutral/input pose，不再回退到错误的 world-origin 转换值。
- 抓取和锁定约束的平滑参考点仍读取当前 limb socket pose，用来保留旧版抓取/释放的流畅感；这个参考点不改变 inactive/default hand target。

现象：
- 大：松开后更快回收，响应更利落，但可能偏硬
- 小：松开后更柔和，但拖尾更长

建议：
- 常用范围：`8 ~ 18`
- 当前默认：`14`

### `ReleaseTargetCompletionDistance`
打开位置：
- 打开 `ABP_ClimbingCharacter`
- 点 `Class Defaults`
- 搜索 `Release Target Completion Distance`
- 分类：`Climbing | Smoothing`

作用：
- release target 距离 neutral/input pose 多近时结束 release 过渡。

建议：
- 当前默认：`0.5cm`
- 如果 release 末尾还有轻微顿挫，可以减小这个值。
- 如果 release 保持过渡太久，可以增大这个值。

注意：
- `ReleaseTargetBlendDuration` 已不再参与当前 release 路径。
- 如果旧参数还出现在 Blueprint 默认值里，关闭 Editor，重新构建 `ClimbEditor`，再打开项目。

---

## 八、ABP_ClimbingCharacter：Pose / Presentation 参数

这些参数只影响表现层，不影响：
- 有没有 yellow candidate
- 哪个点被选中
- 长按能不能锁

另外，`FClimbingLimbAnimTarget` 现在有一个重要状态位：
另外，当前推荐给 Control Rig 使用的是**显式 presentation state**，不要再依赖 target struct 里的瞬态布尔：
- `LeftHandPresentationState`
- `RightHandPresentationState`
- `LeftFootPresentationState`
- `RightFootPresentationState`

枚举值：
- `Inactive`
- `Exploration`
- `Locked`
- `Releasing`

建议：
- CR 用这些状态决定 alpha / 过渡
- target struct 本身只保留 target 几何语义（位置、法线、旋转、locked）

### `HandLockedSurfaceClearance`
作用：
- 手锁住后，沿 target normal 往外推的表面留隙

现象：
- 大：锁住后离黄色球更远
- 小：更贴黄色球

如果你现在要验证“锁定后是否准确到 yellow”，先设成 `0`。

### `HandExplorationSurfaceClearance`
作用：
- 手探索时的表面留隙

当前状态：
- exploration 当前主语义是跟 pink
- 这个值主要作为表现修正，不是核心控制参数

### `FootLockedSurfaceClearance`
作用：
- 脚锁住后的表面留隙

### `FootExplorationSurfaceClearance`
作用：
- 脚探索时的表面留隙

### `HandLockedMaxReach`
作用：
- 手在 locked 状态下允许的最大表现伸展距离

现象：
- 小：更保守，不易扭曲，但可能够不到 yellow
- 大：更容易贴近 yellow，但可能产生夸张姿势

### `HandExplorationMaxReach`
作用：
- 手在 exploration 状态下允许的最大表现伸展距离

现象：
- 小：探索动作更收
- 大：探索动作更大胆，但更容易扭曲/穿模

### `FootLockedMaxReach`
作用：
- 脚在 locked 状态下的最大表现伸展距离

### `FootExplorationMaxReach`
作用：
- 脚在 exploration 状态下的最大表现伸展距离

---

## 九、推荐调参顺序

### 目标：先把探测和抓点逻辑调顺
顺序：
1. `Left/Right Hand/Foot ProbeSettings`
2. `TraceRadius`
3. `SearchDepth`
4. `TargetWallDistance`

### 目标：先验证“锁住是否精确到 yellow”
顺序：
1. `HandLockedSurfaceClearance = 0`
2. `FootLockedSurfaceClearance = 0`
3. 视情况调大 `HandLockedMaxReach / FootLockedMaxReach`
4. 再看 Control Rig 的 `PositionAlpha / StrengthAlpha`

### 目标：改善动作顺滑
顺序：
1. `ExplorationTargetInterpSpeed`
2. `LockedTargetInterpSpeed`
3. `PelvisOffsetInterpSpeed`
4. `TargetRotationInterpSpeed`

### 目标：减少扭曲和穿模
顺序：
1. `HandExplorationMaxReach / FootExplorationMaxReach`
2. `HandLockedMaxReach / FootLockedMaxReach`
3. Control Rig 里的 `PositionAlpha / StrengthAlpha`
4. 最后再调 surface clearance

---

## 十、当前调试语义基线

当前控制语义应该按这个理解：

- 短按激活 limb：显示该 limb 的紫色 reach plane
- 不推 `IJKL`：limb 保持默认，不进入 exploration
- 推 `IJKL`：pink target 在 plane 内移动，limb 跟 pink
- 出现 yellow：只表示这里可锁，不改变 free exploration 姿态
- 长按锁成功：limb 切到 locked target

这套语义如果后面再改，记得同步更新本文档。

---

## 十一、常见现象速查表

这一节只回答一个问题：

**“我现在看到这个现象，第一反应先调哪个参数？”**

### 1. 紫色平面离身体太远 / 太近
优先看：
- `ForwardOffset`
- `VerticalOffset`
- `LateralOffset`

规则：
- 前后不对：先调 `ForwardOffset`
- 高低不对：先调 `VerticalOffset`
- 左右偏心不对：再调 `LateralOffset`

### 2. 紫色平面大小明显超出 limb 可达范围
优先看：
- `HorizontalRange`
- `VerticalRange`

规则：
- 左右范围太宽：减 `HorizontalRange`
- 上下范围太高/太低：减 `VerticalRange`

### 3. 左手去抢右手的点 / 左脚去抢右脚的点
优先看：
- `HorizontalRange`
- `LateralOffset`
- `TraceRadius`

规则：
1. 先缩 `HorizontalRange`
2. 还抢，再小调 `LateralOffset`
3. 还乱吸，再减 `TraceRadius`

### 4. 平面位置对了，但经常没有 yellow candidate
优先看：
- `SearchDepth`
- `TraceRadius`
- `TraceChannel`
- `bTraceComplex`

规则：
1. 先加一点 `SearchDepth`
2. 再适当增大 `TraceRadius`
3. 再检查岩点碰撞是否真的 `Block` 当前 `TraceChannel`
4. 真实 mesh 还是不稳定，再试 `bTraceComplex`

### 5. yellow candidate 经常吸到旁边点，不是当前 limb 最近点
优先看：
- `TraceRadius`
- `HorizontalRange`
- `VerticalRange`
- `LateralOffset`

规则：
1. 先减 `TraceRadius`
2. 再缩 plane 的 `HorizontalRange / VerticalRange`
3. 最后才微调 `LateralOffset`

### 6. pink 动了，但 limb 跟得慢 / 跟不紧
优先看：
- `ExplorationTargetInterpSpeed`
- Control Rig 里的 `PositionAlpha`
- `HandExplorationMaxReach / FootExplorationMaxReach`

规则：
1. 先加 `ExplorationTargetInterpSpeed`
2. 再看 CR 里的 exploration `PositionAlpha`
3. 最后看 exploration `MaxReach` 是否太小

### 7. 锁定后 limb 不够贴近黄色球
优先看：
- `HandLockedSurfaceClearance`
- `FootLockedSurfaceClearance`
- `HandLockedMaxReach`
- `FootLockedMaxReach`
- `LockedTargetInterpSpeed`

规则：
1. 先把 `LockedSurfaceClearance` 设小，甚至临时设 `0`
2. 如果还是够不到，再加大 `LockedMaxReach`
3. 如果只是“慢半拍”，再加 `LockedTargetInterpSpeed`

### 8. 锁定后 limb 明显埋进岩点
优先看：
- `HandLockedSurfaceClearance`
- `FootLockedSurfaceClearance`

规则：
- 埋进表面，就增大 locked clearance

### 9. exploration 时 limb 明显穿进墙或岩点
优先看：
- `HandExplorationSurfaceClearance`
- `FootExplorationSurfaceClearance`
- `HandExplorationMaxReach / FootExplorationMaxReach`

规则：
1. 先适当增大 exploration clearance
2. 如果还是很夸张，再减 exploration max reach

### 10. 大动作时手脚和躯干扭曲很严重
优先看：
- `HandExplorationMaxReach / FootExplorationMaxReach`
- `HandLockedMaxReach / FootLockedMaxReach`
- Control Rig 里的 `StrengthAlpha`
- `LockedTargetInterpSpeed`

规则：
1. 先减 `ExplorationMaxReach`
2. 再减 `LockedMaxReach`
3. 再看 CR 的 alpha 是否太激进
4. 最后才看 smoothing

### 11. exploration -> locked 切换太猛 / 太跳
优先看：
- `LockedTargetInterpSpeed`
- `ExplorationTargetInterpSpeed`
- Control Rig 的 `PositionAlpha / StrengthAlpha`

规则：
1. 先看 locked 侧是不是太快
2. 再看 exploration 侧是不是太硬
3. 最后在 CR 里调状态切换权重

### 12. release 后 limb 弹回太硬
优先看：
- `ExplorationTargetInterpSpeed`
- `PelvisOffsetInterpSpeed`
- CR 里的 inactive alpha 是否回零太猛

注意：
- 这里很多时候不只是 C++ 参数问题，CR 里的状态过渡也会参与

### 13. CoM debug 变化太敏感 / 不敏感
优先看：
- `StableOffsetThreshold`
- `MaxBodyTensionOffset`

规则：
- 太容易判不稳：增大 `StableOffsetThreshold`
- tension 一下就满：增大 `MaxBodyTensionOffset`
- tension 基本没变化：减小 `MaxBodyTensionOffset`

### 14. 贴墙距离整体不对，像悬空或埋墙
优先看：
- `TargetWallDistance`
- `AttachInterpSpeed`

规则：
- 太贴墙：增大 `TargetWallDistance`
- 太离墙：减小 `TargetWallDistance`
- 贴墙过程太生硬/太拖：调 `AttachInterpSpeed`

### 15. 想先验证“逻辑对不对”，暂时不管美术表现
建议临时这样调：
- `HandLockedSurfaceClearance = 0`
- `FootLockedSurfaceClearance = 0`
- 适当增大 `HandLockedMaxReach / FootLockedMaxReach`
- 增大 `ExplorationTargetInterpSpeed`

目的：
- 先看 target、candidate、lock 语义是否正确
- 再把表现约束一点点加回来
## Control Rig 状态桥接说明

当前项目里，`UClimbingAnimInstance` 内部仍然以 `EClimbingLimbPresentationState` 作为状态真相。
但 `ABP -> Control Rig` 这一跳不要再直接消费 enum。当前 UE5.4 / RigVM 组合下，
native enum 输入在 Control Rig 里表现不稳定，常见现象包括：

- CR 里始终读到默认值
- `Equals(enum)` 比较触发 `PropertyA->SameType(PropertyB)` 断言
- 重新编译或重建节点后仍然存在旧类型缓存

因此，推荐给 `CR_ClimbingBody` 的稳定输入是每个 limb 三个 bool：

- `bLeftHandIsExploration / bLeftHandIsLocked / bLeftHandIsReleasing`
- `bRightHandIsExploration / bRightHandIsLocked / bRightHandIsReleasing`
- `bLeftFootIsExploration / bLeftFootIsLocked / bLeftFootIsReleasing`
- `bRightFootIsExploration / bRightFootIsLocked / bRightFootIsReleasing`

`Inactive` 不单独传，直接在 CR 里按下面规则隐式得到：

- `Inactive = !bIsExploration && !bIsLocked && !bIsReleasing`

这样做的分层是：

- C++ / AnimInstance：保留清晰的 enum 状态机
- ABP / Control Rig：只消费稳定的 bool gate
