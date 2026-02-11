// Shared types between frontend and backend

// API Response wrapper
export interface ApiResponse<T> {
  success: boolean;
  data?: T;
  error?: string;
}

// Player/Session types
export interface Player {
  id: string;
  sessionId: string;
  createdAt: string;
}

// Game state types
export interface GameState {
  playerId: string;
  currentLevel: number;
  unlockedMessages: string[];
  completedMinigames: string[];
  discoveredSecrets: string[];
}

// Minigame types
export type MinigameType = 'pattern-match' | 'minesweeper-variant' | 'network-pathfind';

export interface MinigameResult {
  type: MinigameType;
  completed: boolean;
  score: number;
  timeMs: number;
}

// Message types
export interface Message {
  id: string;
  type: 'email' | 'chat';
  from: string;
  subject?: string;
  body: string;
  unlockedAt?: string;
  read: boolean;
}