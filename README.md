# Panopticorp: Neotropolis Event Project

Evil surveillance corporation for cyberpunk event (May 2026).

## Components
- Corporate website with hidden secrets
- Multi-layer hacking game
- Physical vehicle installation (Subaru Baja → RSU)

## Tech Stack
- Frontend: Svelte 5 + Vite + TypeScript
- Backend: Express + TypeScript + SQLite
- Package Manager: pnpm (workspaces)
- Hosting: Self-hosted / VPS

## Timeline
3-month sprint: Feb-May 2026

See `docs/timeline.md` for detailed schedule.

## Project Structure
```
neotropolis-panopticorp/
├── .github/           # Issue templates, CI/CD
├── docs/              # Project documentation
├── assets/            # Brand, copy, vehicle, physical assets
├── web/               # Frontend + backend web app
├── game/              # Minigames, narrative, progression
└── vehicle/           # Electronics, fabrication, content
```

## Getting Started

### Prerequisites
- Node.js >= 20.0.0
- pnpm >= 9.0.0

### Installation
```bash
# Install dependencies
pnpm install

# Copy environment file
cp .env.example .env
```

### Development
```bash
# Run both frontend and backend
pnpm dev

# Or run separately
pnpm dev:frontend  # http://localhost:3000
pnpm dev:backend   # http://localhost:3001
```

### Build
```bash
pnpm build
```

### Project URLs
- Frontend: http://localhost:3000
- Backend API: http://localhost:3001
- Health check: http://localhost:3001/api/health
