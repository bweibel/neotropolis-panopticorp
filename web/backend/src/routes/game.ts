import { Router } from 'express';
import type { ApiResponse, GameState } from '@panopticorp/shared';

export const gameRouter = Router();

// Placeholder: Get current game state
gameRouter.get('/state', (_req, res) => {
  const response: ApiResponse<GameState> = {
    success: true,
    data: {
      playerId: 'placeholder',
      currentLevel: 0,
      unlockedMessages: [],
      completedMinigames: [],
      discoveredSecrets: [],
    },
  };
  res.json(response);
});

// Placeholder: Update game state
gameRouter.post('/state', (req, res) => {
  const response: ApiResponse<{ updated: boolean }> = {
    success: true,
    data: { updated: true },
  };
  res.json(response);
});
