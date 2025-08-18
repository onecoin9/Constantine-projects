# 美食清单应用设计文档

## 概述

美食清单应用是一个基于Web的单页应用（SPA），专门为微信环境优化。采用前端主导的架构，结合云端数据存储，提供流畅的移动端用户体验。应用支持离线查看和在线同步，确保用户随时随地都能管理自己的美食记录。

## 架构

### 整体架构
```
┌─────────────────┐    ┌──────────────────┐
│   微信客户端      │    │   单HTML文件      │
│                │    │                 │
│ - 内置浏览器     │◄──►│ - 原生JavaScript │
│ - 微信JS-SDK    │    │ - CSS样式       │
│ - 分享功能      │    │ - LocalStorage  │
│                │    │ - 响应式设计     │
└─────────────────┘    └──────────────────┘
```

### 技术栈选择

**单文件技术栈：**
- 原生HTML5：结构和内容
- 原生CSS3：样式和响应式布局
- 原生JavaScript (ES6+)：交互逻辑和数据管理
- LocalStorage：数据持久化存储
- 微信JS-SDK：微信功能集成（可选）

## 组件和接口

### HTML结构设计

```html
<!DOCTYPE html>
<html>
<head>
  <!-- 元数据和样式 -->
</head>
<body>
  <div id="app">
    <!-- 头部导航 -->
    <header class="header">
      <h1>美食清单</h1>
      <div class="search-bar"></div>
    </header>
    
    <!-- 分类标签 -->
    <nav class="tab-bar">
      <button class="tab active">待品尝</button>
      <button class="tab">已品尝</button>
    </nav>
    
    <!-- 主要内容区 -->
    <main class="main-content">
      <div class="food-list"></div>
    </main>
    
    <!-- 添加按钮 -->
    <button class="fab">+</button>
    
    <!-- 模态框 -->
    <div class="modal hidden">
      <div class="modal-content">
        <form class="food-form"></form>
      </div>
    </div>
  </div>
  
  <!-- JavaScript逻辑 -->
  <script>
    // 所有应用逻辑
  </script>
</body>
</html>
```

### JavaScript模块设计

**核心功能模块：**
```javascript
// 数据管理
const FoodStorage = {
  save: (foods) => localStorage.setItem('toeatlist', JSON.stringify(foods)),
  load: () => JSON.parse(localStorage.getItem('toeatlist') || '[]'),
  export: () => // 导出数据功能
}

// 界面管理
const UI = {
  renderFoodList: (foods, filter) => {},
  showModal: (type, data) => {},
  hideModal: () => {},
  updateTabState: (activeTab) => {}
}

// 搜索和筛选
const Search = {
  filter: (foods, query, status) => {},
  sort: (foods, sortBy) => {}
}

// 微信集成（可选）
const Wechat = {
  share: (data) => {},
  getLocation: () => {}
}
```

## 数据模型

### 美食记录模型 (LocalStorage)
```javascript
// 存储在 localStorage['toeatlist'] 中的数组
[
  {
    id: String,                 // 唯一ID (时间戳 + 随机数)
    name: String,               // 美食名称
    restaurant: String,         // 餐厅/地点
    category: String,           // 菜系分类 (可选)
    status: String,             // 'to-eat' | 'eaten'
    rating: Number,             // 评分 (1-5, 仅已品尝)
    notes: String,              // 备注
    tags: String,               // 标签 (逗号分隔)
    price: String,              // 价格 (可选)
    createdAt: String,          // 创建时间 (ISO字符串)
    updatedAt: String,          // 更新时间 (ISO字符串)
    eatenAt: String             // 品尝时间 (仅已品尝, ISO字符串)
  }
]
```

### 应用设置模型
```javascript
// 存储在 localStorage['toeatlist_settings'] 中
{
  defaultView: String,          // 'to-eat' | 'eaten'
  sortBy: String,               // 'date' | 'name' | 'rating'
  sortOrder: String,            // 'asc' | 'desc'
  theme: String,                // 'light' | 'dark'
  lastBackup: String            // 最后备份时间
}
```

### 数据导入导出格式
```javascript
// 导出的JSON格式，便于备份和迁移
{
  version: "1.0",
  exportDate: "2025-01-18T10:30:00Z",
  foods: [...],                 // 美食数据数组
  settings: {...}               // 用户设置
}
```

## 错误处理

### 前端错误处理策略

1. **网络错误处理：**
   - 自动重试机制（最多3次）
   - 离线模式降级
   - 用户友好的错误提示

2. **数据验证错误：**
   - 实时表单验证
   - 清晰的错误信息显示
   - 防止重复提交

3. **微信环境错误：**
   - 微信API调用失败处理
   - 分享功能降级方案
   - 授权失败处理

### 后端错误处理

```javascript
// 统一错误响应格式
{
  success: false,
  error: {
    code: 'VALIDATION_ERROR',
    message: '数据验证失败',
    details: {
      field: 'name',
      reason: '美食名称不能为空'
    }
  },
  timestamp: '2025-01-18T10:30:00Z'
}
```

**错误类型定义：**
- `VALIDATION_ERROR`: 数据验证错误
- `AUTHENTICATION_ERROR`: 认证失败
- `AUTHORIZATION_ERROR`: 权限不足
- `NOT_FOUND_ERROR`: 资源不存在
- `DUPLICATE_ERROR`: 数据重复
- `SERVER_ERROR`: 服务器内部错误

## 测试策略

### 前端测试

1. **单元测试 (Vitest)：**
   - 组件逻辑测试
   - 工具函数测试
   - 状态管理测试

2. **集成测试：**
   - API调用测试
   - 路由导航测试
   - 数据流测试

3. **端到端测试 (Playwright)：**
   - 用户操作流程测试
   - 微信环境模拟测试
   - 离线功能测试

### 数据存储测试

1. **LocalStorage测试：**
   - 数据保存和读取测试
   - 数据格式验证测试
   - 存储容量限制测试

2. **数据完整性测试：**
   - 数据导入导出测试
   - 数据迁移测试
   - 异常情况恢复测试

### 微信环境测试

1. **微信开发者工具测试：**
   - 微信内置浏览器兼容性
   - JS-SDK功能测试
   - 分享功能测试

2. **真机测试：**
   - 不同微信版本测试
   - 不同手机型号测试
   - 网络环境测试

## 部署和运维

### 部署架构

```
┌─────────────────┐    ┌──────────────────┐
│   微信分享       │    │   单HTML文件      │
│                │    │                 │
│ - 分享链接     │◄──►│ - 静态文件托管    │
│ - 二维码       │    │ - GitHub Pages   │
│ - 收藏书签     │    │ - Netlify/Vercel │
└─────────────────┘    │ - 或任意Web服务器 │
                      └──────────────────┘
```

### 性能优化

1. **单文件优化：**
   - CSS和JS内联减少请求
   - 图片使用base64编码或外链
   - 代码压缩和混淆
   - 虚拟滚动（大列表）

2. **本地存储优化：**
   - 数据分页加载
   - 延迟渲染非关键内容
   - 定期清理过期数据
   - 数据压缩存储

3. **微信优化：**
   - 预加载关键资源
   - 减少DOM操作
   - 优化触摸事件处理
   - 适配微信内置浏览器

### 安全考虑

1. **数据安全：**
   - HTTPS访问（托管平台自动提供）
   - 本地数据加密存储（可选）
   - 定期数据备份提醒
   - 防止XSS攻击的输入验证

2. **用户隐私：**
   - 数据完全本地存储
   - 无用户追踪和分析
   - 提供数据导出功能
   - 清除数据选项

3. **代码安全：**
   - 输入数据验证和清理
   - 防止恶意脚本注入
   - 安全的DOM操作
   - 错误信息不泄露敏感信息

### 数据备份方案

1. **手动备份：**
   - 导出JSON文件功能
   - 复制到剪贴板
   - 发送到微信文件传输助手

2. **自动提醒：**
   - 定期提醒用户备份
   - 数据量达到阈值时提醒
   - 长时间未备份时提醒