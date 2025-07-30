// --- IMPORTAÇÕES NECESSÁRIAS ---
// Express: Para criar o servidor e as rotas da API.
// sqlite3: O driver para interagir com o banco de dados SQLite.
// cors: Para permitir que nosso arquivo HTML (o frontend) se comunique com este servidor.
const express = require("express");
const sqlite3 = require("sqlite3").verbose();
const cors = require("cors");

// --- CONFIGURAÇÃO INICIAL ---
const app = express();
const PORT = 3000; // A porta onde o servidor irá rodar

// --- MIDDLEWARE ---
app.use(cors()); // Habilita o CORS para todas as rotas
app.use(express.json()); // Permite que o servidor entenda JSON no corpo das requisições

// --- CONEXÃO COM O BANCO DE DADOS ---
// Isso cria um novo arquivo 'controller_logs.db' se ele não existir.
const db = new sqlite3.Database("./controller_logs.db", (err) => {
  if (err) {
    console.error("Erro ao abrir o banco de dados:", err.message);
  } else {
    console.log("Conectado ao banco de dados SQLite.");
    // Cria a tabela de logs se ela ainda não existir.
    db.run(
      `CREATE TABLE IF NOT EXISTS logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            button1 BOOLEAN NOT NULL,
            button2 BOOLEAN NOT NULL,
            joystick_pressed BOOLEAN NOT NULL,
            joystick_x REAL NOT NULL,
            joystick_y REAL NOT NULL
        )`,
      (err) => {
        if (err) {
          console.error("Erro ao criar a tabela:", err.message);
        } else {
          console.log("Tabela 'logs' pronta para uso.");
        }
      }
    );
  }
});

app.get("/", (req, res) => {
  res.send("Hello World!");
});

// --- ROTAS DA API ---

/**
 * Rota GET /logs
 * Retorna todos os registros de log do banco de dados.
 */
app.get("/logs", (req, res) => {
  const sql = "SELECT * FROM logs ORDER BY timestamp DESC";
  db.all(sql, [], (err, rows) => {
    if (err) {
      res.status(500).json({ error: err.message });
      return;
    }
    res.json({
      message: "success",
      data: rows,
    });
  });
});

/**
 * Rota GET /log-get
 * Recebe o estado do controle pelos parâmetros da URL e salva um novo registro.
 * NOTA: Usar GET para criar dados não é uma prática padrão (REST),
 * mas está implementado conforme solicitado.
 */
app.get("/log-get", (req, res) => {
  // Extrai os parâmetros da query da URL
  const { button1, button2, joystick_pressed, joystick_x, joystick_y } =
    req.query;

  // --- CONVERSÃO E VALIDAÇÃO DOS DADOS ---
  // Parâmetros de URL são sempre strings, então precisamos convertê-los.

  // Converte para boolean: 'true' vira true, qualquer outra coisa vira false.
  const p_button1 = button1 === "true";
  const p_button2 = button2 === "true";
  const p_joystick_pressed = joystick_pressed === "true";

  // Converte para número de ponto flutuante (float).
  const p_joystick_x = parseFloat(joystick_x);
  const p_joystick_y = parseFloat(joystick_y);

  // Validação dos dados convertidos. Verifica se os números são válidos.
  if (isNaN(p_joystick_x) || isNaN(p_joystick_y)) {
    return res.status(400).json({
      error:
        "Dados inválidos. Verifique os valores de joystick_x e joystick_y.",
    });
  }

  const sql = `INSERT INTO logs (timestamp, button1, button2, joystick_pressed, joystick_x, joystick_y) VALUES (?, ?, ?, ?, ?, ?)`;
  const params = [
    new Date().toISOString(),
    p_button1,
    p_button2,
    p_joystick_pressed,
    p_joystick_x,
    p_joystick_y,
  ];

  db.run(sql, params, function (err) {
    if (err) {
      res.status(500).json({ error: err.message });
      return;
    }
    res.json({
      message: "Log salvo com sucesso via GET!",
      id: this.lastID,
    });
  });
});

/**
 * Rota POST /log
 * Recebe o estado do controle e salva um novo registro no banco de dados.
 */
app.post("/log", (req, res) => {
  const { button1, button2, joystick_pressed, joystick_x, joystick_y } =
    req.body;

  // Validação simples dos dados recebidos
  if (
    typeof button1 !== "boolean" ||
    typeof button2 !== "boolean" ||
    typeof joystick_pressed !== "boolean" ||
    typeof joystick_x !== "number" ||
    typeof joystick_y !== "number"
  ) {
    return res
      .status(400)
      .json({
        error: "Dados inválidos. Verifique os tipos de dados enviados.",
      });
  }

  const sql = `INSERT INTO logs (timestamp, button1, button2, joystick_pressed, joystick_x, joystick_y) VALUES (?, ?, ?, ?, ?, ?)`;
  const params = [
    new Date().toISOString(),
    button1,
    button2,
    joystick_pressed,
    joystick_x,
    joystick_y,
  ];

  db.run(sql, params, function (err) {
    if (err) {
      res.status(500).json({ error: err.message });
      return;
    }
    res.json({
      message: "Log salvo com sucesso!",
      id: this.lastID,
    });
  });
});

// --- INICIALIZAÇÃO DO SERVIDOR ---
const os = require("os");

app.listen(PORT, () => {
  const interfaces = os.networkInterfaces();
  let localIP = "não encontrado";

  for (const name of Object.keys(interfaces)) {
    for (const iface of interfaces[name]) {
      if (iface.family === "IPv4" && !iface.internal) {
        localIP = iface.address;
        break;
      }
    }
    if (localIP !== "não encontrado") break;
  }

  console.log(`Servidor rodando em:`);
  console.log(`- http://localhost:${PORT}`);
  console.log(`- http://${localIP}:${PORT}`);
});
