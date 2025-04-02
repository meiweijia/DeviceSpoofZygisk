# Device Spoof Zygisk Module

基于 Zygisk 的 Android 设备信息伪装模块，通过 YAML 配置文件实现动态设备信息伪装。修改配置文件后，强行停止目标应用，重新打开即可生效。

## 📁 配置文件说明

### 1. 模板配置文件 `template.yml`
- **路径**: `/data/adb/modules/device_spoof/template.yml`
- **格式说明**:

```yaml
- template: 模板名称
  fields:
    字段名: 值
    字段名: 值
```
示例：
```yaml
- template: galaxy_s23
  fields:
    MODEL: Galaxy S23
    DEVICE: exynos
    MANUFACTURER: Samsung
- template: pixel_9_pro
  fields:
    BOARD: caiman
    BRAND: Google
    DEVICE: caiman
    DISPLAY: AD1A.240530.047.U1 release-keys
    FINGERPRINT: google/caiman/caiman:14/AD1A.240530.047.U1/12150698:user/release-keys
    ID: AD1A.240530.047.U1
    MANUFACTURER: Google
    MODEL: Pixel 9 Pro
    PRODUCT: caiman
    SOC_MANUFACTURER: Google
    SOC_MODEL: Tensor G4
```

### 2. 目标应用配置文件 `target.yml`
- **目标应用配置区路径**: `/data/adb/modules/device_spoof/target.yml`
- **格式说明**:
```yaml
- package: 应用包名
  template: 要使用的模板名称  # 可选
  fields:                  # 可选（直接指定字段）
    字段名: 值
```
示例：
```yaml
# app1 - 使用完整Pixel模板
- package: "com.example.app1"
  template: "pixel_9_pro"

# app2 - 使用三星模板但修改型号
- package: "ccom.example.app2"
  template: "galaxy_s23"
  fields:
    MODEL: "Galaxy S23 Ultra"
    DISPLAY: "TP1A.220624.014.G986USQU4EWE2"

# app3 - 完全自定义配置
- package: "com.example.app3"
  fields:
    MODEL: "Redmi Note 13 Pro"
    MANUFACTURER: "Xiaomi"
    BRAND: "Redmi"
    PRODUCT: "nuwa"
    DEVICE: "nuwa"
    FINGERPRINT: "Redmi/nuwa_global/nuwa:13/TKQ1.221114.001/V14.0.6.0.TMBMIXM:user/release-keys"
```

- fields 支持字段
- [参考 https://developer.android.com/reference/android/os/Build#fields_1](https://developer.android.com/reference/android/os/Build#fields_1 "参考 https://developer.android.com/reference/android/os/Build#fields_1")

```
MODEL
DEVICE
MANUFACTURER
BRAND
PRODUCT
FINGERPRINT
ID
DISPLAY
HARDWARE
SERIAL
```
