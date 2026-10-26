# 🦅 Garuda Web Framework Core Application (Phoenix Alternative)

Aplikasi Web terisolasi *Multi-Worker* berbasis **Garuda Web Framework Core** dengan dukungan *Dynamic Path Parameters (`/users/:id`)*, *JSON REST Controllers*, dan *Database Persistence*.

---

## ⚡ Endpoints Web API

| Method | Path Pattern | Controller | Description |
|--------|--------------|------------|-------------|
| `GET` | `/` | `index_controller/0` | Web Health check & status |
| `GET` | `/users/:id` | `show_user_controller/1` | Dynamic user lookup by ID |
| `POST` | `/users` | `create_user_controller/1` | Create new user record |

---

## 🛠️ Cara Menjalankan

```bash
cd examples/web_app
grdmix run
```
