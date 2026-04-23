# Sanguosha Server with Database Support

这是一个支持用户注册和登录的三国杀服务器。

## 新增功能

### 数据库支持
- 使用SQLite数据库存储用户信息
- 支持用户注册和登录
- 密码使用SHA256哈希存储

### 新增消息类型
- `REGISTER_REQUEST`: 用户注册请求
- `REGISTER_RESPONSE`: 用户注册响应

## 数据库表结构

```sql
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    email TEXT UNIQUE NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_login DATETIME
);
```

## 构建依赖

需要安装以下库：
- SQLite3
- OpenSSL (用于密码哈希)

## 使用方法

1. 启动服务器时会自动创建数据库文件 `sanguosha.db`
2. 客户端可以发送注册请求创建新用户
3. 使用注册的用户名密码进行登录

## API说明

### 注册
客户端发送 `REGISTER_REQUEST` 消息，包含：
- username: 用户名
- password: 密码
- email: 邮箱

服务器返回 `REGISTER_RESPONSE` 消息，包含：
- success: 是否成功
- error_message: 错误信息（如用户名已存在等）
- user_id: 新用户ID

### 登录
客户端发送 `LOGIN_REQUEST` 消息，包含：
- username: 用户名
- password: 密码

服务器返回 `LOGIN_RESPONSE` 消息，包含：
- success: 是否成功
- error_message: 错误信息
- user_id: 用户ID

## 注意事项

当前注册功能暂未完全实现，需要protoc重新生成代码后才能正常工作。登录功能已经可以使用数据库验证。