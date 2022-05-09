import { createSlice, PayloadAction } from '@reduxjs/toolkit';

const initialState = { user: '', brand: '', token: '', image: '' };

interface ActionPayload {
  user: string;
  brand: string;
  token: string;
  image: string;
}

const counterSlice = createSlice({
  name: 'loginUser',
  initialState,
  reducers: {
    setQsData: (state, action: PayloadAction<ActionPayload>) => {
      state.user = action.payload.user;
      state.brand = action.payload.brand;
      state.token = action.payload.token;
      state.image = action.payload.image;
    },
  },
});
export const { setQsData } = counterSlice.actions;
export default counterSlice.reducer;
