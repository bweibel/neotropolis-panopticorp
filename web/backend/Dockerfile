# Build stage
FROM node:20-alpine AS builder

WORKDIR /app

# Install pnpm
RUN corepack enable && corepack prepare pnpm@latest --activate

# Copy workspace files
COPY pnpm-workspace.yaml package.json pnpm-lock.yaml ./
COPY web/shared/package.json ./web/shared/
COPY web/backend/package.json ./web/backend/

# Install dependencies
RUN pnpm install --frozen-lockfile

# Copy source
COPY web/shared/ ./web/shared/
COPY web/backend/ ./web/backend/

# Build shared types first, then backend
RUN pnpm --filter @panopticorp/shared build
RUN pnpm --filter @panopticorp/backend build

# Production stage
FROM node:20-alpine

WORKDIR /app

# Install pnpm for production install
RUN corepack enable && corepack prepare pnpm@latest --activate

# Copy package files
COPY pnpm-workspace.yaml package.json pnpm-lock.yaml ./
COPY web/shared/package.json ./web/shared/
COPY web/backend/package.json ./web/backend/

# Install production dependencies only
RUN pnpm install --frozen-lockfile --prod

# Copy built files
COPY --from=builder /app/web/shared/dist ./web/shared/dist
COPY --from=builder /app/web/backend/dist ./web/backend/dist

# Create data directory for SQLite
RUN mkdir -p /app/data

ENV NODE_ENV=production
ENV PORT=3001

EXPOSE 3001

CMD ["node", "web/backend/dist/index.js"]
