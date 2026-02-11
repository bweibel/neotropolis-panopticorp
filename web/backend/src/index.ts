import express from 'express';
import cors from 'cors';
import { healthRouter } from './routes/health.js';
import { gameRouter } from './routes/game.js';

const app = express();
const PORT = process.env.PORT || 3001;

// Middleware
app.use(cors());
app.use(express.json());

// Routes
app.use('/api/health', healthRouter);
app.use('/api/game', gameRouter);

// Start server
app.listen(PORT, () => {
  console.log(`[Panopticorp] Backend running on http://localhost:${PORT}`);
});
