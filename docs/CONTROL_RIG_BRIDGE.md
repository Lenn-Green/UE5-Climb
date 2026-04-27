# Control Rig 桥接契约

本文档定义当前 Gameplay C++ 与未来 Control Rig 资产之间的明确边界。

## 目的

`AClimbingCharacter`、`UClimbingMovementComponent`、`UClimbingHoldQueryComponent` 和 `UClimbingSolver` 仍然是 Gameplay 权威层。`UClimbingAnimInstance` 负责把最终动画目标整理成稳定的数据包，供 `ABP_ClimbingCharacter` 和后续攀岩 Control Rig 使用，而不是在动画层重复实现 Gameplay 逻辑。

## 当前权威来源

- Gameplay 状态：`AClimbingCharacter`
- 攀爬附着与移动约束：`UClimbingMovementComponent`
- 岩点查询：`UClimbingHoldQueryComponent`
- 纯数学求解：`UClimbingSolver`
- 动画桥接：`UClimbingAnimInstance`

## 当前 Rig 目标契约

`UClimbingAnimInstance` 现在以一个 Blueprint 可读的数据包 `ControlRigTargets` 暴露以下内容：

- `bIsClimbing`
- `ActiveProbeLimb`
- `PelvisOffset`
- `LeftHandTarget`
- `RightHandTarget`
- `LeftFootTarget`
- `RightFootTarget`

每个肢体目标包含：

- `bHasTarget`
- `bIsLocked`
- `TargetLocation`
- `TargetNormal`
- `TargetRotation`
- `LoadPercent`

## 推荐的 Control Rig 变量名

创建 `CR_ClimbingBody` 时，变量名保持与桥接层一致：

- `PelvisOffset`
- `LeftHandTarget`
- `RightHandTarget`
- `LeftFootTarget`
- `RightFootTarget`
- `ActiveProbeLimb`

`ABP_ClimbingCharacter` 应直接读取 `ControlRigTargets` 或继承下来的对应变量，并把这些值喂给 Control Rig 节点或 Rig 变量。不要在 Anim Blueprint 或 Rig 内部重新构建 Gameplay 状态。

## Control Rig 中禁止承担的职责

以下职责不能放进 Control Rig：

- 岩点有效性判断
- Sphere Trace 或任何世界查询
- 攀爬状态切换
- Body Tension / 稳定性求解逻辑
- 抓握按下 / 松开的权威判定

Control Rig 只负责表现层：FBIK 求解、附加位移和视觉修正。

## 下一阶段的接入方向

进入正式 Control Rig 资产阶段后，按这个顺序推进：

1. 创建 `CR_ClimbingBody`
2. 添加与桥接契约一致的变量
3. 在 `ABP_ClimbingCharacter` 中把 `ControlRigTargets` 接入 Control Rig 节点
4. 只用 FBIK 处理手、脚、骨盆的表现
5. Rig 缺失或接线不完整时，系统要能平稳退化，不能让 Gameplay 崩掉

## 当前实现说明

`UClimbingAnimInstance` 现在会先把骨盆偏移和四肢目标转换到 Skeletal Mesh Component Space，再暴露给 `ABP_ClimbingCharacter` 和 `CR_ClimbingBody`。因此，Rig 内部不要再把这些值当作 World Space 重新解释。

## FBIK 第一版配置

第一版 Control Rig 的目标是验证桥接链路、骨盆偏移和双手 Effector 可以稳定协同工作，不是最终的攀岩姿态解算。

### ABP 接线

在 `ABP_ClimbingCharacter` 中：

1. 准备一个稳定的基础 Pose，接入 `Control Rig` 节点
2. 将 `CR_ClimbingBody` 指定为 Rig Class
3. 在节点上暴露并连接以下输入：
   - `PelvisOffset`
   - `ActiveProbeLimb`
   - `LeftHandTarget`
   - `RightHandTarget`
   - `LeftFootTarget`
   - `RightFootTarget`
4. 将 Control Rig 输出接回 `Output Pose`

不要在 Anim Blueprint 里重复创建 `UClimbingAnimInstance` 已经继承下来的同名变量，直接读取继承变量即可。

### Rig 变量

在 `CR_ClimbingBody` 中，创建与桥接契约完全同名、同类型的变量：

- `PelvisOffset`：`Vector`
- `ActiveProbeLimb`：`EClimbingLimb`
- `LeftHandTarget`：`FClimbingLimbAnimTarget`
- `RightHandTarget`：`FClimbingLimbAnimTarget`
- `LeftFootTarget`：`FClimbingLimbAnimTarget`
- `RightFootTarget`：`FClimbingLimbAnimTarget`

### 骨盆阶段

在 FBIK 之前，先把 `PelvisOffset` 作用到骨盆骨上。

- 节点：`Set Translation - Bone`
- Bone：`pelvis`
- Space：`Local Space`
- Weight：`1.0`

这里的几何含义很直接：`PelvisOffset` 已经是局部偏移向量，所以应该让它在当前 Rig 局部坐标系里推动骨盆，而不是把骨盆强行拉到某个全局绝对位置。

### FBIK 阶段

在骨盆平移之后，添加一个 `Full Body IK` 节点。

推荐的第一版初始参数：

- Root：`pelvis`
- 左手 Effector Bone：`hand_l`
- 右手 Effector Bone：`hand_r`
- 左右手 `Chain Depth`：`4`
- `Pull Chain Alpha`：`1.0`
- `Pin Rotation`：`0.0`
- `Rotation Alpha`：`0.0`

第一版只先接手部的平移目标，不要急着开启手部旋转解算。先把身体稳定住，再处理旋转和更细的姿态修正。

### Effector 输入映射

对每只手的 Effector，都按下面映射：

- `Translation` <- `TargetLocation`
- `Position Alpha` <- `IsLocked`
- `Strength Alpha` <- `IsLocked`
- `Rotation Alpha` <- `0.0`

这样可以保证：

- 未锁定的手保持基础 Pose 主导
- 只有 Gameplay 明确锁定到岩点后，该手的 Effector 才会完全生效

### 第一版不要做的事

第一版先不要做下面这些事：

- 把 World Space 目标位置直接喂给 FBIK
- 直接用 `TargetRotation` 强行控制手部旋转
- 在手部还不稳定前就加入脚部 Effector
- 在 Rig 内部重复做选点、Trace 或 Solver 逻辑
- 让 Rig 决定角色是否继续保持攀爬

## 调试顺序

调试 Rig 时，按这个顺序排查：

1. 先确认 `PelvisOffset` 单独就能驱动 `pelvis`
2. 再确认 `LeftHandTarget` 和 `RightHandTarget` 在 Rig 调试器里确实有值变化
3. 临时断开手部 `Translation`，确认网格不再炸开或被拉飞
4. 一次只重新接回一只手的 Effector
5. 单手稳定后，再接双手

这个顺序的意义是把空间错误、链深设置错误和双手耦合问题拆开，避免所有问题混在一起看不清。

## 第一版验收标准

当且仅当以下条件都成立时，第一版 FBIK 才算通过：

- 攀爬时 `PelvisOffset` 能明显带动躯干偏移
- 锁定左手时，左臂会朝左手目标移动
- 锁定右手时，右臂会朝右手目标移动
- 手部目标激活时，网格不会被从 Capsule 位置拉飞
- 解除手部锁定后，该手会回到基础 Pose 主导，而不是继续被钉在空间里

只有这一步稳定通过之后，才应该继续接脚部 Effector、手部旋转和更复杂的攀岩姿态修正。
