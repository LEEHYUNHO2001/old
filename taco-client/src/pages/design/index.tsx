import { NextPage } from 'next';
import React, { useEffect } from 'react';

import { getData } from '@/utils/fetcher/getData';

const Design: NextPage = () => {
  useEffect(() => {
    getData('design');
  }, []);
  return <div>Design</div>;
};

export default Design;
