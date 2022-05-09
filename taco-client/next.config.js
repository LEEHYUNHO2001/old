/** @type {import('next').NextConfig} */
const nextConfig = {
  reactStrictMode: true,
  images: {
    loader: 'akamai',
    path: '/',
  },
  async rewrites() {
    return [
      {
        source: '/api/:path*',
        destination: `http://localhost:9090/:path*`,
      },
    ];
  },
};
module.exports = nextConfig;
