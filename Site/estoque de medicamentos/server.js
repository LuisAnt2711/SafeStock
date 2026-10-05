const express = require('express');
const cors = require('cors');
const path = require('path');
const Database = require('better-sqlite3');

// ---------- Banco de dados ----------
const db = new Database(path.join(__dirname, 'safestock.db'));

db.exec(`
  CREATE TABLE IF NOT EXISTS medications (
    uid        INTEGER PRIMARY KEY AUTOINCREMENT, -- chave interna (uma linha por lote)
    id         TEXT    NOT NULL DEFAULT '',       -- ID do remédio digitado no formulário
    name       TEXT    NOT NULL,
    quantity   INTEGER NOT NULL DEFAULT 0 CHECK (quantity >= 0),
    expiry     DATE    NOT NULL,                  -- AAAA-MM-DD
    fileira    TEXT    NOT NULL DEFAULT ''
  )
`);

// ---------- Servidor ----------
const app = express();
app.use(cors());
app.use(express.json());

function validar(body) {
  const quantity = Number(body.quantity);
  if (!body.name || !String(body.name).trim()) return 'O nome é obrigatório.';
  if (!Number.isInteger(quantity) || quantity < 0) return 'Quantidade inválida.';
  if (!/^\d{4}-\d{2}-\d{2}$/.test(body.expiry || '')) return 'Validade inválida (use AAAA-MM-DD).';
  return null;
}

const buscar = (uid) => db.prepare('SELECT * FROM medications WHERE uid = ?').get(uid);

// Listar
app.get('/api/medications', (req, res) => {
  res.json(db.prepare('SELECT * FROM medications ORDER BY uid').all());
});

// Cadastrar
app.post('/api/medications', (req, res) => {
  const erro = validar(req.body);
  if (erro) return res.status(400).json({ erro });

  const { id = '', name, quantity, expiry, fileira = '' } = req.body;
  const info = db
    .prepare('INSERT INTO medications (id, name, quantity, expiry, fileira) VALUES (?, ?, ?, ?, ?)')
    .run(String(id).trim(), String(name).trim(), Number(quantity), expiry, String(fileira).trim());

  res.status(201).json(buscar(info.lastInsertRowid));
});

// Editar
app.put('/api/medications/:uid', (req, res) => {
  const erro = validar(req.body);
  if (erro) return res.status(400).json({ erro });

  const { id = '', name, quantity, expiry, fileira = '' } = req.body;
  const info = db
    .prepare('UPDATE medications SET id = ?, name = ?, quantity = ?, expiry = ?, fileira = ? WHERE uid = ?')
    .run(String(id).trim(), String(name).trim(), Number(quantity), expiry, String(fileira).trim(), req.params.uid);

  if (info.changes === 0) return res.status(404).json({ erro: 'Medicamento não encontrado.' });
  res.json(buscar(req.params.uid));
});

// Dar baixa (retirar unidades; remove o lote se zerar)
app.patch('/api/medications/:uid/baixa', (req, res) => {
  const med = buscar(req.params.uid);
  if (!med) return res.status(404).json({ erro: 'Medicamento não encontrado.' });

  const qtd = Number(req.body.quantidade);
  if (!Number.isInteger(qtd) || qtd <= 0) return res.status(400).json({ erro: 'Quantidade inválida.' });
  if (qtd > med.quantity) return res.status(400).json({ erro: 'Quantidade maior que o estoque atual.' });

  const novaQtd = med.quantity - qtd;
  if (novaQtd === 0) {
    db.prepare('DELETE FROM medications WHERE uid = ?').run(med.uid);
    return res.json({ removido: true });
  }
  db.prepare('UPDATE medications SET quantity = ? WHERE uid = ?').run(novaQtd, med.uid);
  res.json(buscar(med.uid));
});

// Excluir
app.delete('/api/medications/:uid', (req, res) => {
  const info = db.prepare('DELETE FROM medications WHERE uid = ?').run(req.params.uid);
  if (info.changes === 0) return res.status(404).json({ erro: 'Medicamento não encontrado.' });
  res.json({ removido: true });
});

const PORT = 3000;
app.listen(PORT, () => console.log(`SafeStock API rodando em http://localhost:${PORT}`));
