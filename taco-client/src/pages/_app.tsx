import { Global } from '@emotion/react';
import type { AppProps } from 'next/app';
import { QueryClientProvider } from 'react-query';

import { wrapper } from '@/store';
import { reset } from '@/styles/Reset';
import { queryClient } from '@/utils/query';

function MyApp({ Component, pageProps }: AppProps) {
  return (
    <QueryClientProvider client={queryClient}>
      <Global styles={reset} />
      <Component {...pageProps} />
    </QueryClientProvider>
  );
}

export default wrapper.withRedux(MyApp);
