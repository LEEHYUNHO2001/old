import type { NextPage } from 'next';
import { useEffect } from 'react';

import { getData } from '@/utils/fetcher/getData';

const Home: NextPage = () => {
  useEffect(() => {
    getData();
  }, []);

  return <div>Home</div>;
};

export default Home;
