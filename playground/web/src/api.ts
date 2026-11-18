export async function api<T>(path: string, init?: RequestInit): Promise<T> {
  const response = await fetch('/api' + path, {
    ...init,
    headers: { 'Content-Type': 'application/json', ...init?.headers },
  });
  if (!response.ok) {
    let message = `HTTP ${response.status}`;
    try {
      const data = await response.json();
      message = typeof data.detail === 'string' ? data.detail : JSON.stringify(data.detail);
    } catch {
      /* preserve HTTP status */
    }
    throw new Error(message);
  }
  return response.json() as Promise<T>;
}
export function post<T>(path: string, data: unknown = {}) {
  return api<T>(path, { method: 'POST', body: JSON.stringify(data) });
}
export function errorMessage(error: unknown) {
  return error instanceof Error ? error.message : String(error);
}
export function downloadText(text: string, filename: string) {
  const url = URL.createObjectURL(new Blob([text], { type: 'text/plain;charset=utf-8' }));
  const a = document.createElement('a');
  a.href = url;
  a.download = filename;
  document.body.appendChild(a);
  a.click();
  a.remove();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}
export function saveSetting(key: string, value: unknown) {
  try {
    localStorage.setItem('astercc:' + key, JSON.stringify(value));
  } catch {
    /* editor remains usable without storage */
  }
}
export function loadSetting<T>(key: string, fallback: T): T {
  try {
    const value = localStorage.getItem('astercc:' + key);
    return value === null ? fallback : (JSON.parse(value) as T);
  } catch {
    return fallback;
  }
}
