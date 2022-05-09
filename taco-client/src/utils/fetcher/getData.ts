import axios, { Method } from 'axios';

export const getData = async (endpoint = '') => {
  try {
    const res = await axios({
      method: 'get' as Method,
      url: `/api/${endpoint}`,
    });
    console.log(res.data);
  } catch (error) {
    console.log(error);
  }
};
