# AS

## 编译

```powershell
cd E:\Akie\Assignment\AS
mingw32-make
```

默认模型：`mingw32-make run`（等价于加载 MacBook）。

## 参数说明

| 参数 | 说明 |
|------|------|
| `-model <path/to/scene.gltf>` | 加载指定 glTF 场景；省略时使用 `Models/macbook_air_notebook_pbr/scene.gltf` |
| `-h` / `--help` | 打印用法与可用模型路径 |

GUI：左键旋转；右键/中键平移；滚轮或 `+`/`-` 缩放；`S` 切换 1.0× / 1.5× / 2.0× 超采样；`F` 开关 FXAA；`Q` / `Esc` 退出。

模型放在本地 `Models\`（未纳入仓库）。

## 样例

### macbook_air_notebook_pbr — 默认 PBR + 法线贴图

```powershell
.\pbr_viewer.exe -model Models\macbook_air_notebook_pbr\scene.gltf
```

### chess_set — 清漆 clearcoat

```powershell
.\pbr_viewer.exe -model Models\chess_set\scene.gltf
```

### low_poly_chess_set — 无贴图、仅 Metallic-Roughness 因子

```powershell
.\pbr_viewer.exe -model Models\low_poly_chess_set\scene.gltf
```

### aviator_sunglasses — 玻璃 transmission + BLEND

```powershell
.\pbr_viewer.exe -model Models\aviator_sunglasses\scene.gltf
```

### the_ultimate_glass_pack_cups_and_bottles — transmission 贴图

```powershell
.\pbr_viewer.exe -model Models\the_ultimate_glass_pack_cups_and_bottles\scene.gltf
```

### lmu_main_hall_ceiling_glass_pbr_texture — 大场景玻璃

```powershell
.\pbr_viewer.exe -model Models\lmu_main_hall_ceiling_glass_pbr_texture\scene.gltf
```

### crystal_stone_rock — emissive + BLEND

```powershell
.\pbr_viewer.exe -model Models\crystal_stone_rock\scene.gltf
```

### cosmetic_serum_bottle — 产品级 baseColor

```powershell
.\pbr_viewer.exe -model Models\cosmetic_serum_bottle\scene.gltf
```

### ship_in_a_bottle — transmission + clearcoat

```powershell
.\pbr_viewer.exe -model Models\ship_in_a_bottle\scene.gltf
```

### primogemmorastardust_from_genshin_impact_free — 因子金属 + emissive

```powershell
.\pbr_viewer.exe -model Models\primogemmorastardust_from_genshin_impact_free\scene.gltf
```

### bouquet_de_fleurs_tropicales__new_version_pbr — SpecGloss→MR 自动烘焙

```powershell
.\pbr_viewer.exe -model Models\bouquet_de_fleurs_tropicales__new_version_pbr\scene.gltf
```
